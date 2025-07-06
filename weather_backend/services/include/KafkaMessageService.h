#pragma once

#include <cppkafka/cppkafka.h> // Для cppkafka::Message
#include <nlohmann/json.hpp>   // Для nlohmann::json
#include <string>              // Для std::string
#include <iostream>            // Для std::cout, std::cerr
#include <drogon/drogon.h>

using std::string;

class KafkaMessageService {
public:
    KafkaMessageService();
    void processMessage(const cppkafka::Message& msg);
    void set_DB(const char* name);

private:
    void handleStartCommand(long long telegram_user_id, const string& username, const string& first_name);
    void handleWeatherCommand(const nlohmann::json& payload, const string& rawPayload);
    void handleTelegramMessage(const nlohmann::json& payload, const string& rawPayload);

    drogon::orm::DbClientPtr dbClient_; 
};