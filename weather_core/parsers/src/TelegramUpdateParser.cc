// telegramupdateparser.cc
#include "TelegramUpdateParser.h"

ParsedTelegramMessage TelegramUpdateParser::parse(const std::string& json_payload) {
    ParsedTelegramMessage parsed_msg;
    parsed_msg.is_valid = false; // default to invalid message

    try {
        nlohmann::json kafka_payload = nlohmann::json::parse(json_payload);
        parsed_msg.original_payload = kafka_payload; // store original payload for debugging or future use

        // 1. check for 'event_type'
        if (kafka_payload.contains("event_type") && kafka_payload["event_type"].is_string()) {
            parsed_msg.event_type = kafka_payload["event_type"].get<std::string>();
        } else {
            std::cerr << "parser error: 'event_type' missing or invalid in kafka payload. payload: " << json_payload << std::endl;
            return parsed_msg; 
        }

        // check that event_type is "telegram_command" as sent by the python bot after nlp processing
        if (parsed_msg.event_type != "telegram_command") { 
            std::cerr << "parser error: expected 'event_type' to be 'telegram_command', but got '" 
                      << parsed_msg.event_type << "'. payload: " << json_payload << std::endl;
            return parsed_msg;
        }

        // 2. extract data from the "data" block
        if (kafka_payload.contains("data") && kafka_payload["data"].is_object()) {
            const nlohmann::json& data_obj = kafka_payload["data"];

            // 2.1 extract user data from "data.user"
            if (data_obj.contains("user") && data_obj["user"].is_object()) {
                const nlohmann::json& user_obj = data_obj["user"];
                // tg_user_id
                if (user_obj.contains("user_id") && user_obj["user_id"].is_number_integer()) {
                    parsed_msg.telegram_user_id = user_obj["user_id"].get<long long>();
                } else {
                    std::cerr << "parser error: 'user_id' missing or invalid in 'data.user'. payload: " << json_payload << std::endl;
                    return parsed_msg;
                }
                // username
                if (user_obj.contains("username") && user_obj["username"].is_string()) {
                    parsed_msg.username = user_obj["username"].get<std::string>();
                } else { parsed_msg.username = ""; } // acceptable if username is missing
                // first name
                if (user_obj.contains("first_name") && user_obj["first_name"].is_string()) {
                    parsed_msg.first_name = user_obj["first_name"].get<std::string>();
                } else { parsed_msg.first_name = "unknown"; } // default if first_name is missing
                // chat id
                if (user_obj.contains("chat_id") && user_obj["chat_id"].is_number_integer()) {
                    parsed_msg.chat_id = user_obj["chat_id"].get<long long>();
                } else {
                    std::cerr << "parser error: 'chat_id' missing or invalid in 'data.user'. payload: " << json_payload << std::endl;
                    return parsed_msg;
                }
            } else {
                std::cerr << "parser error: 'user' object missing or invalid in 'data'. payload: " << json_payload << std::endl;
                return parsed_msg;
            }

            // 2.2 extract original_text (for 'text' field in db)
            if (data_obj.contains("original_text") && data_obj["original_text"].is_string()) {
                parsed_msg.original_text = data_obj["original_text"].get<std::string>();
            } else {
                parsed_msg.original_text = ""; // acceptable if text is missing
            }

            // 2.3 extract command (for 'command_text' field in db, now often the nlp-recognized intent)
            if (data_obj.contains("command") && data_obj["command"].is_string()) {
                parsed_msg.command_text = data_obj["command"].get<std::string>();
            } else {
                parsed_msg.command_text = ""; // acceptable if command is missing
            }
            
            // 2. 3extract nlp_entities 
            if (data_obj.contains("entities") && data_obj["entities"].is_object()) {
                parsed_msg.nlp_entities = data_obj["entities"];
            } else {
                parsed_msg.nlp_entities = nlohmann::json::object(); // initialize as empty json object
            }
            
            // attempt to extract city from nlp_entities directly
            if (parsed_msg.nlp_entities.contains("city") && parsed_msg.nlp_entities["city"].is_string()) {
                parsed_msg.command_argument_city = parsed_msg.nlp_entities["city"].get<std::string>();
            }

            parsed_msg.is_valid = true; // if we reached here, the message is considered valid
            
        } else {
            std::cerr << "parser error: 'data' object missing or invalid in kafka payload. payload: " << json_payload << std::endl;
            return parsed_msg;
        }

    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "parser error: json parse error: " << e.what() << ". payload: " << json_payload << std::endl;
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "parser error: json access error: " << e.what() << ". payload: " << json_payload << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "parser error: unexpected error during parsing: " << e.what() << ". payload: " << json_payload << std::endl;
    }

    return parsed_msg;
}

std::string TelegramUpdateParser::extractBaseCommand(const std::string& full_command_text) {
    // finds the first space to separate the base command from arguments
    size_t space_pos = full_command_text.find(' ');
    if (space_pos != std::string::npos) {
        return full_command_text.substr(0, space_pos);
    }
    return full_command_text; // if no space, the whole string is the command
}

std::optional<std::string> TelegramUpdateParser::extractCityArgumentFromCommand(const std::string& full_command_text) {
    // finds the position of the first space
    size_t first_space_pos = full_command_text.find(' ');
    // if no space or space is at the very end, no argument
    if (first_space_pos == std::string::npos || first_space_pos == full_command_text.length() - 1) {
        return std::nullopt; 
    }
    // extract the substring after the first space
    std::string city_arg = full_command_text.substr(first_space_pos + 1);

    // trim leading/trailing whitespace from the extracted argument
    size_t start = city_arg.find_first_not_of(" \t\n\r\f\v");
    size_t end = city_arg.find_last_not_of(" \t\n\r\f\v");
    // if trimming results in an empty string (e.g., only spaces were after command)
    if (std::string::npos == start) return std::nullopt;
    return city_arg.substr(start, end - start + 1);
}