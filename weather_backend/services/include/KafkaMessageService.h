#pragma once

#include <cppkafka/cppkafka.h>
#include <nlohmann/json.hpp>   
#include <string>       
#include <iostream>           
#include <drogon/drogon.h>    
#include <map>                 
#include <memory>  
#include <functional>            

#include "KafkaResponseSender.h"
#include "TelegramUpdateParser.h"
#include "ICommandLogic.h"   

#include <drogon/orm/Mapper.h>
#include "Users.h"
#include "Messages.h"

using std::string;
using std::map;
using std::shared_ptr;

class KafkaMessageService {
public:
    KafkaMessageService();

    void set_OpenWeatherApiKey(const std::string& key);
    void set_DBClientName(const char* name);
    void set_ResponseSender(KafkaResponseSenderPtr response_sender);

    void registerCommandLogic(const string& command_name, ICommandLogicPtr logic);

    void processMessage(const cppkafka::Message& msg);

private:
    drogon::orm::DbClientPtr getDbClient();

    void dispatchCommand(const string& command_name,
                         const nlohmann::json& payload,
                         long long telegram_user_id,
                         const string& message_text,
                         const string& username,
                         const string& first_name);

    void handleUserAndMessage(
        long long telegram_user_id,
        const std::string& username,
        const std::string& first_name,
        const std::string& message_text
    );

    void saveMessageToDb(
        drogon::orm::DbClientPtr dbClient,
        long long telegram_user_id,
        const std::string& message_text
    );

    TelegramUpdateParser messageParser_;
    drogon::orm::DbClientPtr dbClient_;
    string dbClientName_;
    KafkaResponseSenderPtr responseSender_; 
    map<string, ICommandLogicPtr> commandLogics_;
};

using KafkaMessageServicePtr = shared_ptr<KafkaMessageService>;