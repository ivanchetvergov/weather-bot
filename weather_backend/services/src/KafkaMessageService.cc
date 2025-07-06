#include "KafkaMessageService.h"
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <boost/optional.hpp>
#include <utility>

using namespace std;
using namespace drogon_model::weather_backend;

KafkaMessageService::KafkaMessageService() {
    cout << "KafkaMessageService initialized." << endl;
}

void KafkaMessageService::set_DBClientName(const char* name) {
    if (name && !dbClientName_.empty()) {
        cerr << "WARNING: DB client name already set in KafkaMessageService. Overwriting." << endl;
    }
    if (name) {
        dbClientName_ = name;
        cout << "    KafkaMessageService: DB client name set to '" << dbClientName_ << "'." << endl;
    } else {
        cerr << "ERROR: Attempted to set null DB client name." << endl;
    }
}

drogon::orm::DbClientPtr KafkaMessageService::getDbClient() {
    if (!dbClient_) { 
        if (dbClientName_.empty()) {
            cerr << "ERROR: DB client name is not set in KafkaMessageService. Cannot get client." << endl;
            return nullptr;
        }
        dbClient_ = drogon::app().getDbClient(dbClientName_);
        if (!dbClient_) {
            cerr << "ERROR: Failed to get DbClient '" << dbClientName_ << "' from Drogon app. Check config/startup." << endl;
        } else {
            cout << "    KafkaMessageService: Successfully obtained DbClient '" << dbClientName_ << "'." << endl;
        }
    }
    return dbClient_;
}

void KafkaMessageService::set_ResponseSender(KafkaResponseSenderPtr response_sender) {
    if (!responseSender_) {
        responseSender_ = response_sender;
        cout << "    KafkaMessageService: Response sender set." << endl;
    } else {
        cout << "    KafkaMessageService: Response sender already set (re-setting ignored)." << endl;
    }
}

void KafkaMessageService::registerCommandLogic(const string& command_name, ICommandLogicPtr logic) {
    if (!logic) {
        cerr << "ERROR: Attempted to register null ICommandLogicPtr for '" << command_name << "'." << endl;
        return;
    }
    if (commandLogics_.count(command_name) > 0) {
        cerr << "WARNING: Command logic for '" << command_name << "' already registered. Overwriting." << endl;
    }
    commandLogics_[command_name] = logic;
    cout << "    KafkaMessageService: Registered logic for command: '" << command_name << "'" << endl;
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
        cout << "\n  Received message for Kafka Key: " << kafka_key_str << endl;
    } else {
        cout << "  Received message with no Kafka Key." << endl;
        kafka_key_str = "unknown_kafka_key"; 
    }
    long long telegram_user_id = 0;
    string username = "";
    string first_name = "Unknown";
    string message_text = "";
    string payload_str = msg.get_payload();

    try {
        json kafka_payload = json::parse(payload_str);

        string event_type = "";
        if (kafka_payload.contains("event_type") && kafka_payload["event_type"].is_string()) {
            event_type = kafka_payload["event_type"].get<string>();
            cout << "    Event Type: " << event_type << endl;
        }

        if (event_type == "telegram_message") {
            if (kafka_payload.contains("data") && kafka_payload["data"].is_object() &&
                kafka_payload["data"].contains("message") && kafka_payload["data"]["message"].is_object()) {

                const json& message_obj = kafka_payload["data"]["message"];
                
                if (message_obj.contains("from") && message_obj["from"].is_object() &&
                    message_obj["from"].contains("id") && message_obj["from"]["id"].is_number_integer()) {
                    telegram_user_id = message_obj["from"]["id"].get<long long>();
                    cout << "      Telegram User ID: " << telegram_user_id << endl;
                } else {
                    cerr << "      WARNING: Telegram User ID not found in message payload!" << endl;
                }

                if (message_obj.contains("from") && message_obj["from"].is_object() &&
                    message_obj["from"].contains("username") && message_obj["from"]["username"].is_string()) {
                    username = message_obj["from"]["username"].get<string>();
                    cout << "      Username: " << username << endl;
                } else {
                    cout << "      Username not found." << endl;
                }

                if (message_obj.contains("from") && message_obj["from"].is_object() &&
                    message_obj["from"].contains("first_name") && message_obj["from"]["first_name"].is_string()) {
                    first_name = message_obj["from"]["first_name"].get<string>();
                    cout << "      First Name: " << first_name << endl;
                } else {
                    cerr << "      WARNING: First Name not found for Telegram user!" << endl;
                }

                if (message_obj.contains("text") && message_obj["text"].is_string()) {
                    message_text = message_obj["text"].get<string>();
                    cout << "      Message Text: " << message_text << endl;

                    if (message_text == "/start") {
                        dispatchCommand("/start", kafka_payload, telegram_user_id, message_text, username, first_name);
                    } else if (message_text == "/weather") {
                        dispatchCommand("/weather", kafka_payload, telegram_user_id, message_text, username, first_name);
                    } else {
                        dispatchCommand("telegram_message_general", kafka_payload, telegram_user_id, message_text, username, first_name);
                    }
                } else {
                    dispatchCommand("telegram_message_general", kafka_payload, telegram_user_id, message_text, username, first_name);
                }

            } else {
                cerr << "    WARNING: 'data' or 'message' object missing/invalid in 'telegram_message' event. Dispatching to general handler." << endl;
                dispatchCommand("telegram_message_general", kafka_payload, telegram_user_id, message_text, username, first_name);
            }
        }
        else if (event_type == "/weather_api_response") {
            dispatchCommand("/weather_api_response", kafka_payload, telegram_user_id, message_text, username, first_name);
        }
        else {
            cout << "    --> Unknown or unhandled event type: " << event_type << endl;
            if (responseSender_ && telegram_user_id != 0) {
                 responseSender_->sendTelegramMessage(telegram_user_id, "Извините, я получил неизвестный тип события.");
            }
        }

    } catch (const json::parse_error& e) {
        cerr << "  JSON parse error: " << e.what() << ". Payload: " << payload_str << endl;
    } catch (const json::exception& e) {
        cerr << "  JSON access error: " << e.what() << ". Payload: " << payload_str << endl;
    }
}

void KafkaMessageService::handleUserAndMessage(
    long long telegram_user_id,
    const std::string& username,
    const std::string& first_name,
    const std::string& message_text
) {
    if (!dbClient_) {
        dbClient_ = getDbClient();
        if (!dbClient_) {
             cerr << "ERROR: DbClient is null in handleUserAndMessage. Cannot save user/message." << endl;
             return;
        }
    }

    Mapper<Users> userMapper(dbClient_);
    userMapper.findByPrimaryKey(telegram_user_id,
        [=](Users user) {
            bool changed = false;
            if (user.isUsernameUsed()) {
                if (user.getValueOfUsername() != username) {
                    user.setUsername(username);
                    changed = true;
                }
            } else if (!username.empty()) {
                user.setUsername(username);
                changed = true;
            }

            if (user.getValueOfFirstName() != first_name) {
                user.setFirstName(first_name);
                changed = true;
            }

            if (changed) {
                userMapper.update(user,
                    [=](const size_t count) {
                        cout << "User " << telegram_user_id << " updated." << endl;
                    },
                    [=](const DrogonDbException& e) {
                        cerr << "Error updating user " << telegram_user_id << ": " << e.what() << endl;
                    });
            }
            // Вызов нового общего метода для сохранения сообщения
            saveMessageToDb(dbClient_, telegram_user_id, message_text);
        },
        [=](const DrogonDbException& e) {
            if (e.base().code() == PGSQL_NOT_FOUND || e.base().code() == MYSQL_NOT_FOUND || string(e.what()).find("not found") != string::npos) {
                Users newUser;
                newUser.setId(telegram_user_id);
                newUser.setFirstName(first_name);
                if (!username.empty()) {
                    newUser.setUsername(username);
                } else {
                    newUser.setUserNameAsNull();
                }

                userMapper.insert(newUser,
                    [=](Users insertedUser) {
                        cout << "New user " << telegram_user_id << " inserted." << endl;
                        // Вызов нового общего метода для сохранения сообщения
                        saveMessageToDb(dbClient_, telegram_user_id, message_text);
                    },
                    [=](const DrogonDbException& e_insert) {
                        cerr << "Error inserting new user " << telegram_user_id << ": " << e_insert.what() << endl;
                    });
            } else {
                cerr << "Error finding user " << telegram_user_id << ": " << e.what() << endl;
            }
        });
}

void KafkaMessageService::saveMessageToDb(
    drogon::orm::DbClientPtr dbClient,
    long long telegram_user_id,
    const std::string& message_text
) {
    Mapper<Messages> messageMapper(dbClient);
    Messages newMessage;
    newMessage.setUserId(telegram_user_id);
    newMessage.setText(message_text);

    messageMapper.insert(newMessage,
        [=](Messages insertedMessage) {
            cout << "Message for user " << telegram_user_id << " saved to DB." << endl;
        },
        [=](const DrogonDbException& e) {
            cerr << "Error saving message for user " << telegram_user_id << ": " << e.what() << endl;
        });
}

void KafkaMessageService::dispatchCommand(const string& command_name,
                                        const nlohmann::json& payload,
                                        long long telegram_user_id,
                                        const string& message_text,
                                        const string& username,
                                        const string& first_name) {
    auto it = commandLogics_.find(command_name);
    if (it != commandLogics_.end() && it->second) {
        cout << "    KafkaMessageService: Dispatching to logic for '" << command_name << "'" << endl;
        it->second->execute(getDbClient(),
                            payload,
                            telegram_user_id,
                            message_text,
                            username,
                            first_name);
    } else {
        cerr << "ERROR: No logic registered for command: '" << command_name << "'" << endl;
        if (responseSender_ && telegram_user_id != 0) {
            responseSender_->sendTelegramMessage(telegram_user_id, "Извините, эта команда пока не поддерживается или произошла внутренняя ошибка.");
        }
    }
}