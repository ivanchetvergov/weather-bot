#pragma once
#include <string>
#include <optional>          
#include <nlohmann/json.hpp> 
#include <drogon/utils/TzOffset.h>

using std::string;
using std::optional;

struct UserData {
    long long telegram_user_id;
    string username;
    string first_name;
};

struct MessageData {
    long long user_id;
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
    long long telegram_user_id = 0;
    string username;
    string first_name;
    string message_text;
    string event_type;
    nlohmann::json original_payload; 

    bool is_valid = false; 
};
