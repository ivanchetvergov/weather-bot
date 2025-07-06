// include/ParsedTelegramMessage.h
#pragma once

#include <string>
#include <nlohmann/json.hpp> 

struct ParsedTelegramMessage {
    long long telegram_user_id = 0;
    std::string username;
    std::string first_name;
    std::string message_text;
    std::string event_type;
    nlohmann::json original_payload; 

    bool is_valid = false; 
};