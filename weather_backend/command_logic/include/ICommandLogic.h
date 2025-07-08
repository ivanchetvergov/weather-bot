#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <drogon/drogon.h>

#include "DataBaseService.h"
#include "KafkaResponseSender.h"

using std::string;

class ICommandLogic {
public:
    virtual ~ICommandLogic() = default;

    virtual void execute(const nlohmann::json& payload,
                         long long telegram_user_id,
                         const string& message_text,
                         const string& username,
                         const string& first_name) = 0;
};

using ICommandLogicPtr = shared_ptr<ICommandLogic>;