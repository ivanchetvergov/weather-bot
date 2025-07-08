#pragma once

#include <cppkafka/cppkafka.h>
#include <nlohmann/json.hpp>   
#include <string>       
#include <iostream>           
#include <map>                 
#include <memory>  
         
#include "KafkaResponseSender.h"
#include "TelegramUpdateParser.h"
#include "ICommandLogic.h"   
#include "DataBaseService.h"

using std::string;
using std::map;
using std::shared_ptr;

class KafkaMessageService {
public:
    KafkaMessageService();

    void set_ResponseSender(KafkaResponseSenderPtr response_sender);
    void set_DbService(PgDbServicePtr db_service);

    void registerCommandLogic(const string& command_name, ICommandLogicPtr logic);

    void processMessage(const cppkafka::Message& msg);

private:
    void dispatchCommand(const string& command_name,
                         const nlohmann::json& payload,
                         long long telegram_user_id,
                         const string& message_text,
                         const string& username,
                         const string& first_name);

    void handleTelegramMessage(const ParsedTelegramMessage& parsed_msg);
    
    void handleWeatherApiResponse(const ParsedTelegramMessage& parsed_msg);

    TelegramUpdateParser messageParser_;
    PgDbServicePtr dbService_;
    KafkaResponseSenderPtr responseSender_; 
    map<string, ICommandLogicPtr> commandLogics_;
};

using KafkaMessageServicePtr = shared_ptr<KafkaMessageService>;