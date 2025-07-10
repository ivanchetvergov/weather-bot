// MyCityCommand.h
#pragma once

#include "ICommandLogic.h"
#include "KafkaResponseSender.h"
#include "DataBaseService.h" 

class MyCityCommandLogic : public ICommandLogic {
public:

    MyCityCommandLogic(KafkaResponseSenderPtr response_sender, PgDbServicePtr dbService);

    void execute(const nlohmann::json& payload,
                 long long telegram_user_id,
                 const std::string& message_text, 
                 const std::string& username,
                 const std::string& first_name) override;

private:
    KafkaResponseSenderPtr responseSender_;
    PgDbServicePtr dbService_;

    std::string extractCityArgument(const std::string& full_command_text);
};