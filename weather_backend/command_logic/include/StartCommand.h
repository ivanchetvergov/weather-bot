#pragma once

#include "ICommandLogic.h"
#include <drogon/drogon.h>
#include <string>
#include <memory>

using namespace std;

class StartCommandLogic : public ICommandLogic {
public:

    StartCommandLogic(KafkaResponseSenderPtr response_sender);

    void execute(PgDbServicePtr db_service, 
                const nlohmann::json& payload,
                long long telegram_user_id,
                const string& message_text,
                const string& username,
                const string& first_name) override;

private:
    KafkaResponseSenderPtr responseSender_;
};