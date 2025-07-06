#include "KafkaMessageService.h"
#include <iostream>
#include <string>           
#include <nlohmann/json.hpp>
#include <boost/optional.hpp> 
#include <utility>        
#include <cstdlib>       

using namespace std;

KafkaMessageService::KafkaMessageService() {
    cout << "KafkaMessageService initialized." << endl;
}

void KafkaMessageService::set_DB(const char* name){
    dbClient_ = drogon::app().getDbClient(name);
    if (!dbClient_) {
        cerr << "    ERROR: Failed to get 'default' database client during handleStartCommand! Check config.json." << endl;
        return; 
    }
}

void KafkaMessageService::processMessage(const cppkafka::Message& msg) {
    using json = nlohmann::json;

    string kafka_key_str;
    const auto& temp_optional = msg.get_key();

    if (temp_optional) { 
        // !!! can be dangerous !!!
        const cppkafka::Buffer* key_buffer_ptr = reinterpret_cast<const cppkafka::Buffer*>(&temp_optional);
        const char* key_data = reinterpret_cast<const char*>(key_buffer_ptr->get_data());
        size_t key_size = key_buffer_ptr->get_size();

        kafka_key_str = string(key_data, key_size);
        cout << "  Received message for Kafka Key: " << kafka_key_str << endl;
    } else {
        cout << "  Received message with no Kafka Key." << endl;
        kafka_key_str = "unknown_kafka_key"; 
    }

    string payload_str = msg.get_payload(); 
    // cout << "  Payload (Raw JSON): " << payload_str << endl;

    try {
        json kafka_payload = json::parse(payload_str);

        // cout << "  Parsed JSON fields:" << endl;        
        if (kafka_payload.contains("event_type") && kafka_payload["event_type"].is_string()) {
            string event_type = kafka_payload["event_type"];
            cout << "    Event Type: " << event_type << endl;

            if (event_type == "/weather") {
                handleWeatherCommand(kafka_payload, payload_str);
            }
            // Logic for /start and telegram_message needs to be integrated properly
            else if (event_type == "telegram_message") {
                // parse Telegram message data to extract user info and text
                if (kafka_payload.contains("data") && kafka_payload["data"].is_object() &&
                    kafka_payload["data"].contains("message") && kafka_payload["data"]["message"].is_object()) {
                    
                    const json& message_obj = kafka_payload["data"]["message"];

                    long long telegram_user_id = 0;
                    if (message_obj.contains("from") && message_obj["from"].is_object() &&
                        message_obj["from"].contains("id") && message_obj["from"]["id"].is_number_integer()) {
                        telegram_user_id = message_obj["from"]["id"].get<long long>();
                        cout << "      Telegram User ID: " << telegram_user_id << endl;
                    } else {
                        cerr << "      WARNING: Telegram User ID not found in message payload!" << endl;
                    }

                    string username = "";
                    if (message_obj.contains("from") && message_obj["from"].is_object() &&
                        message_obj["from"].contains("username") && message_obj["from"]["username"].is_string()) {
                        username = message_obj["from"]["username"].get<string>();
                        cout << "      Username: " << username << endl;
                    } else {
                        cout << "      Username not found." << endl;
                    }

                    string first_name = "";
                    if (message_obj.contains("from") && message_obj["from"].is_object() &&
                        message_obj["from"].contains("first_name") && message_obj["from"]["first_name"].is_string()) {
                        first_name = message_obj["from"]["first_name"].get<string>();
                        cout << "      First Name: " << first_name << endl;
                    } else {
                        first_name = "Unknown"; 
                        cerr << "      WARNING: First Name not found for Telegram user!" << endl;
                    }

                    // Check for command text within the Telegram message
                    if (message_obj.contains("text") && message_obj["text"].is_string()) {
                        string message_text = message_obj["text"].get<string>();
                        cout << "      Message Text: " << message_text << endl;

                        if (message_text == "/start" && telegram_user_id != 0) {
                            handleStartCommand(telegram_user_id, username, first_name);
                        } else {
                            handleTelegramMessage(kafka_payload, payload_str);
                        }
                    } else {
                        handleTelegramMessage(kafka_payload, payload_str);
                    }

                } else {
                    cerr << "    WARNING: 'data' or 'message' object missing/invalid in telegram_message event." << endl;
                    handleTelegramMessage(kafka_payload, payload_str); 
                }
            }
            else {
                cout << "    --> Unknown event type: " << event_type << endl;
            }
        } else {
            cout << "    Event type not found or not a string." << endl;
        }
    } catch (const json::parse_error& e) {
        cerr << "  JSON parse error: " << e.what() << ". Payload: " << payload_str << endl;
    }
}

void KafkaMessageService::handleWeatherCommand(const nlohmann::json& payload, const string& rawPayload) {
    using json = nlohmann::json;
    if (payload.contains("data") && payload["data"].is_object()) {
        json data_obj = payload["data"];
        
        if (data_obj.contains("bot_response") && data_obj["bot_response"].is_string()) {
            string bot_response_text = data_obj["bot_response"].get<string>();
            cout << "      Bot Response (Decoded): " << bot_response_text << endl;
        } else {
            cout << "      Bot Response not found or not a string." << endl;
        }
        // if (data_obj.contains("city") && data_obj["city"].is_string()) {
        //     cout << "      City: " << data_obj["city"].get<string>() << endl;
        // }
    } else {
        cout << "    'data' object not found in /weather command payload." << endl;
    }
}

void KafkaMessageService::handleTelegramMessage(const nlohmann::json& payload, const string& rawPayload) {
    cout << "    --> This is a general Telegram message." << endl;
    if (!dbClient_) set_DB("default");

    if (payload.contains("data") && payload["data"].is_object() &&
        payload["data"].contains("message") && payload["data"]["message"].is_object() &&
        payload["data"]["message"].contains("text") && payload["data"]["message"]["text"].is_string()) {
        string message_text = payload["data"]["message"]["text"].get<string>();
        cout << "      Message Text: " << message_text << endl;
    } else {
        cout << "    Message text not found in general Telegram message payload." << endl;
    }
}

void KafkaMessageService::handleStartCommand(long long telegram_user_id, const string& username, const string& first_name) {
    cout << "    --> Handling /start command for Telegram User ID: " << telegram_user_id << endl;

    if (telegram_user_id == 0) { // Telegram user IDs are positive non-zero
        cerr << "    ERROR: Cannot register user with invalid Telegram User ID (0)." << endl;
        return;
    }

    if (!dbClient_) set_DB("default");

    dbClient_->execSqlAsync(
        "INSERT INTO users (id, username, first_name) VALUES ($1, $2, $3) "
        "ON CONFLICT (id) DO UPDATE SET username = EXCLUDED.username, first_name = EXCLUDED.first_name, created_at = CURRENT_TIMESTAMP",
        
        // success callback
        [telegram_user_id, username, first_name](const drogon::orm::Result& result) {
            cout << "    User " << telegram_user_id
                      << " (" << (!username.empty() ? username : "N/A")
                      << ", " << first_name << ") registered/updated successfully in DB." << endl;
        },

        // error callback
        [telegram_user_id](const drogon::orm::DrogonDbException& e) {
            cerr << "    ERROR: Failed to register/update user " << telegram_user_id
                      << " in DB: " << e.base().what() << endl;
        },

        // sql parameters
        telegram_user_id,
        username.empty() ? nullptr : username.c_str(),
        first_name
    );
}
