// TelegramUpdateParser.cc
#include "TelegramUpdateParser.h"
#include <iostream>

ParsedTelegramMessage TelegramUpdateParser::parse(const std::string& json_payload) {
    ParsedTelegramMessage parsed_msg;
    parsed_msg.is_valid = false; // По умолчанию сообщение невалидно

    try {
        nlohmann::json kafka_payload = nlohmann::json::parse(json_payload);
        parsed_msg.original_payload = kafka_payload; 

        if (kafka_payload.contains("event_type") && kafka_payload["event_type"].is_string()) {
            parsed_msg.event_type = kafka_payload["event_type"].get<std::string>();
        } else {
            std::cerr << "Parser Error: 'event_type' missing or invalid in Kafka payload." << std::endl;
            return parsed_msg; // is_valid остается false
        }

        if (parsed_msg.event_type == "telegram_message") {
            if (kafka_payload.contains("data") && kafka_payload["data"].is_object() &&
                kafka_payload["data"].contains("message") && kafka_payload["data"]["message"].is_object()) {

                const nlohmann::json& message_obj = kafka_payload["data"]["message"];

                if (message_obj.contains("from") && message_obj["from"].is_object() &&
                    message_obj["from"].contains("id") && message_obj["from"]["id"].is_number_integer()) {
                    parsed_msg.telegram_user_id = message_obj["from"]["id"].get<long long>();
                } else {
                    std::cerr << "Parser Error: Telegram User ID not found in message payload for 'telegram_message'." << std::endl;
                    return parsed_msg; // is_valid остается false
                }

                if (message_obj.contains("from") && message_obj["from"].is_object() &&
                    message_obj["from"].contains("username") && message_obj["from"]["username"].is_string()) {
                    parsed_msg.username = message_obj["from"]["username"].get<std::string>();
                } // else username remains empty, which is acceptable

                if (message_obj.contains("from") && message_obj["from"].is_object() &&
                    message_obj["from"].contains("first_name") && message_obj["from"]["first_name"].is_string()) {
                    parsed_msg.first_name = message_obj["from"]["first_name"].get<std::string>();
                } else {
                    std::cerr << "Parser Warning: First Name not found for Telegram user. Using 'Unknown'." << std::endl;
                    parsed_msg.first_name = "Unknown";
                }

                if (message_obj.contains("text") && message_obj["text"].is_string()) {
                    parsed_msg.message_text = message_obj["text"].get<std::string>();
                } 
                // else message_text remains empty, also acceptable if no text is sent

                parsed_msg.is_valid = true; // Успешно распарсили Telegram-сообщение
            } else {
                std::cerr << "Parser Error: 'data' or 'message' object missing/invalid in 'telegram_message' event. Payload: " << json_payload << std::endl;
            }
        } 
        else if (parsed_msg.event_type == "telegram_callback_query") { 

            parsed_msg.is_valid = true; 
        }
        else if (parsed_msg.event_type == "/weather_api_response") { 
            parsed_msg.is_valid = true; 
        }
        else {
            std::cerr << "Parser Error: Unknown or unsupported 'event_type': '" << parsed_msg.event_type << "' in Kafka payload. Not parsing as valid Telegram message. Payload: " << json_payload << std::endl;
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