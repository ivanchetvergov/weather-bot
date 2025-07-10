// TelegramUpdateParser.cc
#include "TelegramUpdateParser.h"
#include <iostream>
#include <string>

ParsedTelegramMessage TelegramUpdateParser::parse(const std::string& json_payload) {
    ParsedTelegramMessage parsed_msg;
    parsed_msg.is_valid = false; // По умолчанию сообщение невалидно

    try {
        nlohmann::json kafka_payload = nlohmann::json::parse(json_payload);
        parsed_msg.original_payload = kafka_payload; 

        // Проверяем наличие event_type
        if (kafka_payload.contains("event_type") && kafka_payload["event_type"].is_string()) {
            parsed_msg.event_type = kafka_payload["event_type"].get<std::string>();
        } else {
            std::cerr << "Parser Error: 'event_type' missing or invalid in Kafka payload." << std::endl;
            return parsed_msg; 
        }

        // Проверяем, что event_type - это именно "telegram_message"
        if (parsed_msg.event_type != "telegram_message") {
            std::cerr << "Parser Error: Expected 'event_type' to be 'telegram_message', but got '" 
                      << parsed_msg.event_type << "'. Payload: " << json_payload << std::endl;
            return parsed_msg;
        }

        // 2. Извлечение данных из блока "data"
        if (kafka_payload.contains("data") && kafka_payload["data"].is_object()) {
            const nlohmann::json& data_obj = kafka_payload["data"];

            // 2.1 Извлечение данных пользователя из "data.user"
            if (data_obj.contains("user") && data_obj["user"].is_object()) {
                const nlohmann::json& user_obj = data_obj["user"];

                if (user_obj.contains("user_id") && user_obj["user_id"].is_number_integer()) {
                    parsed_msg.telegram_user_id = user_obj["user_id"].get<long long>();
                } else {
                    std::cerr << "Parser Error: 'user_id' missing or invalid in 'data.user'. Payload: " << json_payload << std::endl;
                    return parsed_msg;
                }

                if (user_obj.contains("username") && user_obj["username"].is_string()) {
                    parsed_msg.username = user_obj["username"].get<std::string>();
                } else { parsed_msg.username = ""; } // Acceptable if username is missing

                if (user_obj.contains("first_name") && user_obj["first_name"].is_string()) {
                    parsed_msg.first_name = user_obj["first_name"].get<std::string>();
                } else { parsed_msg.first_name = "Unknown"; } // Default if first_name is missing

                if (user_obj.contains("chat_id") && user_obj["chat_id"].is_number_integer()) {
                    parsed_msg.chat_id = user_obj["chat_id"].get<long long>();
                } else {
                    std::cerr << "Parser Error: 'chat_id' missing or invalid in 'data.user'. Payload: " << json_payload << std::endl;
                    return parsed_msg;
                }
            } else {
                std::cerr << "Parser Error: 'user' object missing or invalid in 'data'. Payload: " << json_payload << std::endl;
                return parsed_msg;
            }

            // 2.2 Извлечение original_text (для поля 'text' в БД)
            if (data_obj.contains("original_text") && data_obj["original_text"].is_string()) {
                parsed_msg.original_text = data_obj["original_text"].get<std::string>();
            } else {
                parsed_msg.original_text = ""; // Acceptable if text is missing
            }

            // 2.3 Извлечение command (для поля 'command_text' в БД)
            if (data_obj.contains("command") && data_obj["command"].is_string()) {
                parsed_msg.command_text = data_obj["command"].get<std::string>();
            } else {
                parsed_msg.command_text = ""; // Acceptable if command is missing (e.g., just text)
            }
            
            parsed_msg.is_valid = true; // Если дошли сюда, сообщение валидно
            
        } else {
            std::cerr << "Parser Error: 'data' object missing or invalid in Kafka payload. Payload: " << json_payload << std::endl;
            return parsed_msg;
        }

    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Parser Error: JSON parse error: " << e.what() << ". Payload: " << json_payload << std::endl;
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Parser Error: JSON access error: " << e.what() << ". Payload: " << json_payload << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Parser Error: Unexpected error during parsing: " << e.what() << ". Payload: " << json_payload << std::endl;
    }

    return parsed_msg;
}

std::string TelegramUpdateParser::extractBaseCommand(const std::string& full_command_text) {
    size_t space_pos = full_command_text.find(' ');
    if (space_pos != std::string::npos) {
        return full_command_text.substr(0, space_pos);
    }
    return full_command_text; // Если пробела нет, это уже сама команда
}