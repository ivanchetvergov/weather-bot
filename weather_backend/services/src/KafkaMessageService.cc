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
    
    string payload_str = msg.get_payload();
    ParsedTelegramMessage parsed_msg = messageParser_.parse(payload_str);

    if (!parsed_msg.is_valid) {
        cerr << "  ERROR: Failed to parse Kafka message payload. Skipping processing." << endl;
        if (parsed_msg.telegram_user_id != 0 && responseSender_) {
            responseSender_->sendTelegramMessage(parsed_msg.telegram_user_id, "Извините, не удалось понять ваше сообщение. Пожалуйста, попробуйте еще раз.");
        }
        return;
    }

    cout << endl << "    Event Type: " << parsed_msg.event_type << endl;

    if (parsed_msg.event_type == "telegram_message") {
        cout << "      Telegram User ID: " << parsed_msg.telegram_user_id << endl;
        cout << "      Username: " << parsed_msg.username << endl;
        cout << "      First Name: " << parsed_msg.first_name << endl;
        cout << "      Message Text: " << parsed_msg.message_text << endl;
        
        handleUserAndMessage(
            parsed_msg.telegram_user_id,
            parsed_msg.username,
            parsed_msg.first_name,
            parsed_msg.message_text
        );

        if (parsed_msg.message_text.rfind("/start", 0) == 0) {
            dispatchCommand("/start", parsed_msg.original_payload, parsed_msg.telegram_user_id, parsed_msg.message_text, parsed_msg.username, parsed_msg.first_name);
        } else if (parsed_msg.message_text.rfind("/weather", 0) == 0) {
            dispatchCommand("/weather", parsed_msg.original_payload, parsed_msg.telegram_user_id, parsed_msg.message_text, parsed_msg.username, parsed_msg.first_name);
        } else {
            dispatchCommand("telegram_message_general", parsed_msg.original_payload, parsed_msg.telegram_user_id, parsed_msg.message_text, parsed_msg.username, parsed_msg.first_name);
        }
    }
    else if (parsed_msg.event_type == "/weather_api_response") {
        dispatchCommand("/weather_api_response", parsed_msg.original_payload, parsed_msg.telegram_user_id, parsed_msg.message_text, parsed_msg.username, parsed_msg.first_name);
    }
    else {
        cout << "    --> Unknown or unhandled event type: " << parsed_msg.event_type << endl;
        if (responseSender_ && parsed_msg.telegram_user_id != 0) {
             responseSender_->sendTelegramMessage(parsed_msg.telegram_user_id, "Извините, я получил неизвестный тип события.");
        }
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