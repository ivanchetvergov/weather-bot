#include "KafkaMessageService.h"
#include <iostream>
#include <string>           
#include <nlohmann/json.hpp>
#include <boost/optional.hpp> 
#include <utility>          // Для std::move

KafkaMessageService::KafkaMessageService() {
    std::cout << "KafkaMessageService initialized." << std::endl;
}

void KafkaMessageService::processMessage(const cppkafka::Message& msg) {
    using json = nlohmann::json;

    std::string user_id_str;
    std::string payload_str = msg.get_payload(); 

    // --- ПОСЛЕДНИЙ, ОПАСНЫЙ ОБХОДНОЙ ПУТЬ С reinterpret_cast ---
    // Цель: полностью обойти все механизмы boost::optional и получить Buffer* напрямую.
    
    // Получаем временный объект boost::optional<cppkafka::Buffer>.
    // Компилятор с ним постоянно ошибается, поэтому мы будем работать с его сырой памятью.
    const auto& temp_optional = msg.get_key(); // Используем const&, чтобы не пытаться копировать optional_key_buffer

    if (temp_optional) { // Проверяем, что optional содержит значение
        // !!! ОЧЕНЬ ОПАСНЫЙ ХАК !!!
        // Пытаемся "вытащить" указатель на внутренний объект Buffer
        // путем переинтерпретации адреса optional'а.
        // Это зависит от внутренней реализации boost::optional
        // и может не работать или привести к неопределенному поведению.
        const cppkafka::Buffer* key_buffer_ptr = reinterpret_cast<const cppkafka::Buffer*>(&temp_optional);
        
        // Теперь используем полученный указатель на Buffer
        // для доступа к данным через get_data() и get_size().
        // Это должно избежать любых проблем с копированием Buffer.
        const char* key_data = reinterpret_cast<const char*>(key_buffer_ptr->get_data());
        size_t key_size = key_buffer_ptr->get_size();

        user_id_str = std::string(key_data, key_size);
        
        std::cout << "  Received message for User ID (Key): " << user_id_str << std::endl;
    } else {
        std::cout << "  Received message with no Key." << std::endl;
        user_id_str = "unknown_user";
    }
    // --- КОНЕЦ ОПАСНОГО ОБХОДНОГО ПУТИ ---

    std::cout << "  Payload (Raw JSON): " << payload_str << std::endl;

    try {
        json kafka_payload = json::parse(payload_str);

        std::cout << "  Parsed JSON fields:" << std::endl;
        
        if (kafka_payload.contains("event_type") && kafka_payload["event_type"].is_string()) {
            std::string event_type = kafka_payload["event_type"];
            std::cout << "    Event Type: " << event_type << std::endl;

            if (event_type == "/weather") {
                handleWeatherCommand(kafka_payload, payload_str);
            } else if (event_type == "telegram_message") {
                handleTelegramMessage(kafka_payload, payload_str);
            } else {
                std::cout << "    --> Unknown event type." << std::endl;
            }
        } else {
            std::cout << "    Event type not found or not a string." << std::endl;
        }
        
    } catch (const json::parse_error& e) {
        std::cerr << "  JSON parse error: " << e.what() << ". Payload: " << payload_str << std::endl;
    } catch (const json::exception& e) {
        std::cerr << "  JSON access error: " << e.what() << ". Payload: " << payload_str << std::endl;
    }
}

void KafkaMessageService::handleWeatherCommand(const nlohmann::json& payload, const std::string& rawPayload) {
    using json = nlohmann::json;
    std::cout << "    --> This is a /weather command!" << std::endl;
    if (payload.contains("data") && payload["data"].is_object()) {
        json data_obj = payload["data"];
        
        if (data_obj.contains("bot_response") && data_obj["bot_response"].is_string()) {
            std::string bot_response_text = data_obj["bot_response"].get<std::string>();
            std::cout << "      Bot Response (Decoded): " << bot_response_text << std::endl;
        } else {
            std::cout << "      Bot Response not found or not a string." << std::endl;
        }
        if (data_obj.contains("city") && data_obj["city"].is_string()) {
            std::cout << "      City: " << data_obj["city"].get<std::string>() << std::endl;
        }
    } else {
        std::cout << "    'data' object not found in /weather command payload." << std::endl;
    }
}

void KafkaMessageService::handleTelegramMessage(const nlohmann::json& payload, const std::string& rawPayload) {
    std::cout << "    --> This is a general Telegram message." << std::endl;
    if (payload.contains("data") && payload["data"].is_object() &&
        payload["data"].contains("message") && payload["data"]["message"].is_object() &&
        payload["data"]["message"].contains("text") && payload["data"]["message"]["text"].is_string()) {
        std::string message_text = payload["data"]["message"]["text"].get<std::string>();
        std::cout << "      Message Text: " << message_text << std::endl;
    } else {
        std::cout << "    Message text not found in general Telegram message payload." << std::endl;
    }
}d