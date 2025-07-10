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
};
