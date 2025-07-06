#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <drogon/drogon.h>

#include "KafkaResponseSender.h"

using KafkaResponseSenderPtr = std::shared_ptr<KafkaResponseSender>;
using std::string;

class ICommandLogic {
public:
    virtual ~ICommandLogic() = default;

    virtual void execute(drogon::orm::DbClientPtr db_client, 
                         const nlohmann::json& payload,
                         long long telegram_user_id,
                         const string& message_text,
                         const string& username,
                         const string& first_name) = 0;
};

using ICommandLogicPtr = shared_ptr<ICommandLogic>;