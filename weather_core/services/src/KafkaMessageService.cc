#include "KafkaMessageService.h"

#include <utility>                  // for std::move
#include <trantor/utils/Date.h>     // for trantor::Date
#include "DataTransferObjects.h"    // for UserData, MessageData, ParsedTelegramMessage, WeatherCacheData
#include <thread>                   // for std::thread (async processing)

using namespace std;
using namespace drogon_model::weather_bot; // assuming drogon models
using nlohmann::json;

KafkaMessageService::KafkaMessageService() {
    cout << "KafkaMessageService initialized." << endl;
    // define which commands explicitly require a city argument
    commandsRequiringCity_.insert("/weather");
    commandsRequiringCity_.insert("/forecast");
    commandsRequiringCity_.insert("/mycity");
    commandsRequiringCity_.insert("/subscribe"); 
    commandsRequiringCity_.insert("/track");   
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
    ParsedTelegramMessage parsed_msg = messageParser_.parse(payload_str); // use the new parser

    if (!parsed_msg.is_valid) {
        cerr << "  ERROR: Failed to parse Kafka message payload or it was invalid. Skipping processing. Payload: " << payload_str << endl;
        if (parsed_msg.telegram_user_id != 0 && responseSender_) {
            responseSender_->sendTelegramMessage(parsed_msg.telegram_user_id, "Извините, не удалось понять ваше сообщение. Пожалуйста, попробуйте еще раз.");
        }
        return;
    }

    cout << endl << "    Event Type: " << parsed_msg.event_type << endl;

    // the python bot now always sends 'telegram_command' after nlp processing
    if (parsed_msg.event_type == "telegram_command") { 
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

void KafkaMessageService::handleWeatherApiResponse(const ParsedTelegramMessage& parsed_msg) { 
    cout << "      Processing Weather API Response for Telegram User ID: " << parsed_msg.telegram_user_id << endl;

    if (!dbService_) {
        cerr << "ERROR: PgDbService is not set. Cannot process weather_api_response." << endl;
        return;
    }

    // directly use parsed_msg.original_payload as it holds the raw kafka data for this event type
    if (parsed_msg.original_payload.count("city") &&
        parsed_msg.original_payload.count("timestamp") &&
        parsed_msg.original_payload.count("json_data"))
    {
        WeatherCacheData cache_data;
        cache_data.city = parsed_msg.original_payload["city"].get<std::string>();
        cache_data.timestamp = trantor::Date::fromDbString(parsed_msg.original_payload["timestamp"].get<std::string>());
        cache_data.json_data = parsed_msg.original_payload["json_data"];

        dbService_->upsertWeatherCache(cache_data); 
        cout << "    KafkaMessageService: Weather data cached for city: " << cache_data.city << endl;
    } else {
        cerr << "WARNING: Missing 'city', 'timestamp', or 'json_data' in /weather_api_response payload. Cannot cache weather." << endl;
    }

    dispatchCommand("/weather_api_response", parsed_msg.original_payload,
                        parsed_msg.telegram_user_id, parsed_msg.original_text,
                        parsed_msg.username, parsed_msg.first_name);
}

void KafkaMessageService::dispatchCommand(const string& command_name,
                                        const nlohmann::json& payload,
                                        long long telegram_user_id,
                                        const string& message_text, // this is now command_text_for_db_and_dispatch
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

void KafkaMessageService::handleTelegramMessage(const ParsedTelegramMessage& parsed_msg) {
    std::cout << "      Telegram User ID: " << parsed_msg.telegram_user_id << std::endl;
    std::cout << "      Username: " << parsed_msg.username << std::endl;
    std::cout << "      First Name: " << parsed_msg.first_name << std::endl;
    std::cout << "      Original Text: " << parsed_msg.original_text << std::endl;
    std::cout << "      Command/Intent: " << parsed_msg.command_text << std::endl;
    std::cout << "      NLP Entities: " << parsed_msg.nlp_entities.dump() << std::endl; // <-- log entities

    if (!dbService_) {
        std::cerr << "ERROR: PgDbService is not set in KafkaMessageService. Cannot save user/message." << std::endl;
        sendErrorMessageToUser(parsed_msg.telegram_user_id, "Извините, произошла внутренняя ошибка базы данных.");
        return;
    }

    auto state = std::make_shared<AsyncProcessingState>(parsed_msg, this);
    // detach the thread to not block the kafka consumer
    std::thread([state]() {
        try {
            state->service_ptr->step1_upsertUser(state);
            state->service_ptr->step2_resolveCityAndCheckRequirements(state);
            state->service_ptr->step3_insertMessageAndPreparePayload(state);
            state->service_ptr->step4_dispatchCommand(state);
        } catch (const std::exception& e) {
            std::cerr << "ERROR in Telegram message processing chain for user " << state->parsed_msg.telegram_user_id << ": " << e.what() << std::endl;
            state->service_ptr->sendErrorMessageToUser(state->parsed_msg.telegram_user_id, "Извините, произошла внутренняя ошибка при обработке вашей команды.");
        }
    }).detach();
}

void KafkaMessageService::step1_upsertUser(std::shared_ptr<AsyncProcessingState> state) {
    UserData user_data;
    user_data.telegram_user_id = state->parsed_msg.telegram_user_id;
    user_data.username = state->parsed_msg.username;
    user_data.first_name = state->parsed_msg.first_name;

    std::future<void> upsert_fut = state->service_ptr->dbService_->upsertUser(user_data);
    upsert_fut.get(); // wait for the db operation to complete
    std::cout << "    KafkaMessageService: User upserted successfully for ID: " << user_data.telegram_user_id << std::endl;
}

void KafkaMessageService::step2_resolveCityAndCheckRequirements(std::shared_ptr<AsyncProcessingState> state) {
    // getCityForCommand (from text or default user's)
    std::future<std::optional<std::string>> get_city_fut = state->service_ptr->getCityForCommand(
        state->parsed_msg.telegram_user_id, state->parsed_msg.nlp_entities); 
    state->resolved_city = get_city_fut.get(); // get the resolved city

    const std::string command_name = state->parsed_msg.command_text; // command_text is already the base intent from nlp

    // check if the command requires a city and if one was not resolved
    bool city_is_required_for_command = state->service_ptr->commandsRequiringCity_.count(command_name) > 0;

    if (city_is_required_for_command && (!state->resolved_city.has_value() || state->resolved_city.value().empty())) {
        std::cout << "    KafkaMessageService: Command '" << command_name << "' requires city, but no city resolved for user " << state->parsed_msg.telegram_user_id << std::endl;
        state->service_ptr->sendErrorMessageToUser(state->parsed_msg.telegram_user_id,
                                                   "Город не указан в команде и не установлен по умолчанию. Пожалуйста, укажите город. Пример: " + command_name + " Москва");
        throw std::runtime_error("City required but not resolved for command: " + command_name);
    }
    if (state->resolved_city.has_value() && !state->resolved_city.value().empty()) {
        std::cout << "    KafkaMessageService: City resolved: " << state->resolved_city.value() << std::endl;
    }
}

void KafkaMessageService::step3_insertMessageAndPreparePayload(std::shared_ptr<AsyncProcessingState> state) {
    MessageData message_data;
    message_data.user_id = state->parsed_msg.telegram_user_id;
    message_data.text = state->parsed_msg.original_text;

    // the command_text from parsed_msg is already the nlp-recognized intent
    const std::string base_nlp_command = state->parsed_msg.command_text;

    // if a city was resolved and the command requires it, append it for db/dispatch clarity
    bool city_is_required_for_command = state->service_ptr->commandsRequiringCity_.count(base_nlp_command) > 0;
    if (city_is_required_for_command && state->resolved_city.has_value() && !state->resolved_city.value().empty()) {
        state->command_text_for_db_and_dispatch = base_nlp_command + " " + state->resolved_city.value();
        std::cout << "    KafkaMessageService: Formed command_text for DB and dispatch: '" << state->command_text_for_db_and_dispatch << "'" << std::endl;
    } else {
        // otherwise, use the nlp-recognized command as is
        state->command_text_for_db_and_dispatch = base_nlp_command;
        std::cout << "    KafkaMessageService: Using original NLP-recognized command_text for DB and dispatch: '" << state->command_text_for_db_and_dispatch << "'" << std::endl;
    }

    message_data.command_text = state->command_text_for_db_and_dispatch; 
    std::cout << "    KafkaMessageService: Saving command_text to DB: '" << message_data.command_text << "'" << std::endl;

    std::future<void> insert_msg_fut = state->service_ptr->dbService_->insertMessage(message_data);
    insert_msg_fut.get(); // wait for the db operation to complete
    std::cout << "    KafkaMessageService: User message saved to DB for user ID: " << message_data.user_id << std::endl;
}

void KafkaMessageService::step4_dispatchCommand(std::shared_ptr<AsyncProcessingState> state) {
    const std::string base_command_for_dispatch = state->parsed_msg.command_text;

    // pass the original_payload as it contains the full nlp_entities object
    state->service_ptr->dispatchCommand(base_command_for_dispatch, 
                                       state->parsed_msg.original_payload, // <-- pass original_payload with nlp_entities
                                       state->parsed_msg.telegram_user_id,
                                       state->command_text_for_db_and_dispatch, // for db/logging consistency
                                       state->parsed_msg.username,
                                       state->parsed_msg.first_name);
}

std::future<std::optional<std::string>> KafkaMessageService::getCityForCommand(long long telegram_user_id, const nlohmann::json& nlp_entities) {
    // 1. try to get city from nlp entities first
    if (nlp_entities.contains("city") && nlp_entities["city"].is_string() && !nlp_entities["city"].get<std::string>().empty()) {
        std::string city_from_nlp = nlp_entities["city"].get<std::string>();
        std::promise<std::optional<std::string>> prom;
        prom.set_value(city_from_nlp);
        std::cout << "    KafkaMessageService: City found in NLP entities: " << city_from_nlp << std::endl;
        return prom.get_future();
    } 
    // 2. if not found in nlp entities, try to get user's default city from db
    else {
        if (!dbService_) {
            std::promise<std::optional<std::string>> prom;
            prom.set_exception(std::make_exception_ptr(std::runtime_error("Database service is not set. Cannot get default city.")));
            return prom.get_future();
        }

        auto prom = std::make_shared<std::promise<std::optional<std::string>>>();

        dbService_->getUserDefaultCity(telegram_user_id,
            [prom, telegram_user_id](std::optional<std::string> default_city_opt) {
                if (default_city_opt.has_value()) {
                    std::cout << "    KafkaMessageService: Default city found from DB for user " << telegram_user_id << ": " << default_city_opt.value() << std::endl;
                } else {
                    std::cout << "    KafkaMessageService: No default city found from DB for user " << telegram_user_id << std::endl;
                }
                prom->set_value(default_city_opt);
            },
            [prom, telegram_user_id](const std::exception& e) {
                std::cerr << "ERROR: getUserDefaultCity failed for user " << telegram_user_id << ": " << e.what() << std::endl;
                prom->set_exception(std::make_exception_ptr(e));
            }
        );
        return prom->get_future();
    }
}

void KafkaMessageService::sendErrorMessageToUser(long long telegram_user_id, const std::string& error_message) {
    if (responseSender_ && telegram_user_id != 0) {
        responseSender_->sendTelegramMessage(telegram_user_id, error_message);
    }
}