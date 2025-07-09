// ForecastCommand.h
#pragma once

#include "ICommandLogic.h" 
#include "KafkaResponseSender.h" 
#include "DataBaseService.h"     
#include <drogon/HttpClient.h>
#include <nlohmann/json.hpp>
#include <string>
#include <memory> 

class ForecastCommandLogic : public ICommandLogic {
public:
    ForecastCommandLogic(KafkaResponseSenderPtr sender, 
        PgDbServicePtr dbService, const std::string& openWeatherApiKey);

    void execute(const nlohmann::json& payload,
                 long long telegram_user_id,
                 const std::string& message_text,
                 const std::string& username,
                 const std::string& first_name) override;

private:
    void getForecastData(const std::string& city, long long telegram_user_id, 
        const std::string& original_message_text);
        
    void sendErrorMessage(long long telegram_user_id, 
        const std::string& error_message);

    KafkaResponseSenderPtr responseSender_;
    PgDbServicePtr dbService_;
    std::string openWeatherApiKey_;
};