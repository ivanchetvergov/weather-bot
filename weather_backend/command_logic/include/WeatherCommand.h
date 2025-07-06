#pragma once

#include "ICommandLogic.h"
#include "KafkaResponseSender.h" 
#include <drogon/HttpClient.h>   
#include <nlohmann/json.hpp>

#include <drogon/drogon.h>

using std::string;

class WeatherCommandLogic : public ICommandLogic {
public:
    WeatherCommandLogic(KafkaResponseSenderPtr sender, const string& openWeatherApiKey);

    void execute(
        drogon::orm::DbClientPtr dbClient,
        const nlohmann::json& payload,
        long long telegram_user_id,
        const string& message_text,
        const string& username,
        const string& first_name
    ) override;

    void saveMessageToDb(
    drogon::orm::DbClientPtr dbClient,
    long long telegram_user_id,
    const std::string& message_text
    )

private:
    KafkaResponseSenderPtr responseSender_;
    string openWeatherApiKey_;

    void getWeatherData(
        const string& city,
        long long telegram_user_id,
        const string& original_message_text
    );

    void sendErrorMessage(long long telegram_user_id, const string& error_message);
};