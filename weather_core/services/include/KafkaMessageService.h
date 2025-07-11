// KafkaMessageService.h
#pragma once

#include <cppkafka/cppkafka.h> 
#include <nlohmann/json.hpp>   
#include <string>              
#include <iostream>            
#include <map>                 
#include <memory>              
#include <optional>            
#include <future>              
#include <set>                 

#include "KafkaResponseSender.h" 
#include "TelegramUpdateParser.h" 
#include "ICommandLogic.h"       
#include "DataBaseService.h"     

class KafkaMessageService {
public:
    KafkaMessageService();

    void set_ResponseSender(KafkaResponseSenderPtr response_sender);
    void set_DbService(PgDbServicePtr db_service);

    void registerCommandLogic(const std::string& command_name, ICommandLogicPtr logic);

    void processMessage(const cppkafka::Message& msg);

private:

    struct AsyncProcessingState {
            ParsedTelegramMessage parsed_msg;
            std::optional<std::string> resolved_city;
            nlohmann::json enriched_payload;
            std::string final_message_text_for_command;
            KafkaMessageService* service_ptr;

            std::string command_text_for_db_and_dispatch; 

            AsyncProcessingState(const ParsedTelegramMessage& msg, KafkaMessageService* svc)
                : parsed_msg(msg), service_ptr(svc) {}
    };

    void dispatchCommand(const std::string& command_name,
                         const nlohmann::json& payload,
                         long long telegram_user_id,
                         const std::string& message_text,
                         const std::string& username,
                         const std::string& first_name);

    void handleTelegramMessage(const ParsedTelegramMessage& parsed_msg);
    void handleWeatherApiResponse(const ParsedTelegramMessage& parsed_msg);

    std::future<std::optional<std::string>> getCityForCommand(long long telegram_user_id, 
                                                            const std::string& message_text);

    void sendErrorMessageToUser(long long telegram_user_id, const std::string& error_message);

    void processTelegramMessageAsync(std::shared_ptr<AsyncProcessingState> state);

    void step1_upsertUser(std::shared_ptr<AsyncProcessingState> state);
    void step2_resolveCityAndCheckRequirements(std::shared_ptr<AsyncProcessingState> state);
    void step3_insertMessageAndPreparePayload(std::shared_ptr<AsyncProcessingState> state);
    void step4_dispatchCommand(std::shared_ptr<AsyncProcessingState> state);

    TelegramUpdateParser messageParser_;
    PgDbServicePtr dbService_;
    KafkaResponseSenderPtr responseSender_;
    std::map<std::string, ICommandLogicPtr> commandLogics_;
    std::set<std::string> commandsRequiringCity_;
};

using KafkaMessageServicePtr = std::shared_ptr<KafkaMessageService>;