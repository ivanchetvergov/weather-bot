#include "KafkaMessageService.h"

#include <utility>
#include <trantor/utils/Date.h>
#include "DataTransferObjects.h"

using namespace std;
using namespace drogon_model::weather_bot;
using nlohmann::json;

KafkaMessageService::KafkaMessageService() {
    cout << "KafkaMessageService initialized." << endl;
}

void KafkaMessageService::set_ResponseSender(KafkaResponseSenderPtr response_sender) {
    if (!responseSender_) {
        responseSender_ = response_sender;
        cout << "    KafkaMessageService: Response sender set." << endl;
    } else {
        cout << "    KafkaMessageService: Response sender already set (re-setting ignored)." << endl;
    }
}

void KafkaMessageService::set_DbService(PgDbServicePtr db_service) {
    if (!db_service) {
        std::cerr << "ERROR: Attempted to set null PgDbServicePtr in KafkaMessageService." << std::endl;
        return;
    }
    dbService_ = db_service;
    std::cout << "    KafkaMessageService: PgDbService set." << std::endl;
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
        handleTelegramMessage(parsed_msg);
    }
    else if (parsed_msg.event_type == "/weather_api_response") {
        handleWeatherApiResponse(parsed_msg);
    }
    else {
        cout << "    --> Unknown or unhandled event type: " << parsed_msg.event_type << endl;
        if (responseSender_ && parsed_msg.telegram_user_id != 0) {
             responseSender_->sendTelegramMessage(parsed_msg.telegram_user_id, "Извините, я получил неизвестный тип события.");
        }
    }
}

void KafkaMessageService::handleTelegramMessage(const ParsedTelegramMessage& parsed_msg) {
    cout << "      Telegram User ID: " << parsed_msg.telegram_user_id << endl;
    cout << "      Username: " << parsed_msg.username << endl;
    cout << "      First Name: " << parsed_msg.first_name << endl;
    cout << "      Message Text: " << parsed_msg.message_text << endl;

    if (!dbService_) {
        cerr << "ERROR: PgDbService is not set in KafkaMessageService. Cannot save user/message." << endl;
        if (responseSender_ && parsed_msg.telegram_user_id != 0) {
            responseSender_->sendTelegramMessage(parsed_msg.telegram_user_id, "Извините, произошла внутренняя ошибка базы данных.");
        }
        return;
    }

    UserData user_data;
    user_data.telegram_user_id = parsed_msg.telegram_user_id;
    user_data.username = parsed_msg.username;
    user_data.first_name = parsed_msg.first_name;

    dbService_->upsertUser(user_data);

    MessageData message_data;
    message_data.user_id = parsed_msg.telegram_user_id;
    message_data.text = parsed_msg.message_text;

    dbService_->insertMessage(message_data);

    if (parsed_msg.message_text.rfind("/start", 0) == 0) {
        dispatchCommand("/start", parsed_msg.original_payload,
                        parsed_msg.telegram_user_id, parsed_msg.message_text,
                        parsed_msg.username, parsed_msg.first_name);
    } else if (parsed_msg.message_text.rfind("/weather", 0) == 0) {
        dispatchCommand("/weather", parsed_msg.original_payload,
                        parsed_msg.telegram_user_id, parsed_msg.message_text,
                        parsed_msg.username, parsed_msg.first_name);
    } else {
        dispatchCommand("telegram_message_general", parsed_msg.original_payload,
                        parsed_msg.telegram_user_id, parsed_msg.message_text,
                        parsed_msg.username, parsed_msg.first_name);
    }
}

void KafkaMessageService::handleWeatherApiResponse(const ParsedTelegramMessage& parsed_msg) {
    cout << "      Processing Weather API Response for Telegram User ID: " << parsed_msg.telegram_user_id << endl;

    if (!dbService_) {
        cerr << "ERROR: PgDbService is not set. Cannot process weather_api_response." << endl;
        return;
    }

    if (parsed_msg.original_payload.count("city") &&
        parsed_msg.original_payload.count("timestamp") &&
        parsed_msg.original_payload.count("json_data"))
    {
        WeatherCacheData cache_data;
        cache_data.city = parsed_msg.original_payload["city"].get<std::string>();
        cache_data.timestamp = trantor::Date::fromDbString(parsed_msg.original_payload["timestamp"].get<std::string>());
        cache_data.json_data = parsed_msg.original_payload["json_data"];

        dbService_->upsertWeatherCache(cache_data); 
    } else {
        cerr << "WARNING: Missing 'city', 'timestamp', or 'json_data' in /weather_api_response payload. Cannot cache weather." << endl;
    }

    dispatchCommand("/weather_api_response", parsed_msg.original_payload,
                        parsed_msg.telegram_user_id, parsed_msg.message_text,
                        parsed_msg.username, parsed_msg.first_name);
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
        it->second->execute(payload, telegram_user_id,
                            message_text, username, first_name);
    } else {
        cerr << "ERROR: No logic registered for command: '" << command_name << "'" << endl;
        if (responseSender_ && telegram_user_id != 0) {
            responseSender_->sendTelegramMessage(telegram_user_id, "Извините, эта команда пока не поддерживается или произошла внутренняя ошибка.");
        }
    }
}