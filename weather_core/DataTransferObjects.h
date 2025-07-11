#pragma once
#include <string>
#include <optional>          
#include <nlohmann/json.hpp> 
#include <drogon/HttpAppFramework.h>

using std::string;
using std::optional;

struct UserData {
    long long telegram_user_id;
    string username;
    string first_name;
    optional<string> default_city;
};

struct MessageData {
    long long user_id;
    string command_text;
    string text;
};

struct SubscriptionData {
    long long user_id;
    string city;
    optional<float> temp_above; 
    optional<bool> rain;
    optional<float> wind_speed_gt;
    optional<string> notify_time; 
};

struct AlertData {
    long long user_id;
    string city;
    string condition; 
};

struct WeatherCacheData {
    string city;
    trantor::Date timestamp; 
    nlohmann::json json_data;
};

struct ParsedTelegramMessage {
    bool is_valid;
    std::string event_type;
    long long telegram_user_id;
    std::string username;
    std::string first_name;
    long long chat_id; 

    std::string command_text; 
    std::string original_text; 

    nlohmann::json original_payload; 
    std::optional<std::string> command_argument_city;
    
    nlohmann::json nlp_entities; 
};

struct ParsedWeatherConditions {
    std::string city;
    std::optional<float> tempAbove;
    std::optional<float> tempBelow;
    std::optional<bool> rainExpected; 
    std::optional<bool> snowExpected;
    std::optional<float> windSpeedGt;
    std::optional<float> windSpeedLt;
    std::optional<float> humidityGt;
    std::optional<float> humidityLt;
    std::optional<float> pressureGt;
    std::optional<float> pressureLt;

    std::optional<std::chrono::hours> notifyIntervalHours; 
    std::optional<std::chrono::minutes> notifyTimeMinutes; 
                                                         
    std::optional<bool> notifyOnChange; 
    std::optional<std::string> detailLevel;
    std::optional<std::string> units;      

    Json::Value nlpEntities; 
