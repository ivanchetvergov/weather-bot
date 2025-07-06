#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <iostream> 

using namespace string;

struct ParsedTelegramMessage {
    long long telegram_user_id = 0;
    string username;
    string first_name;
    string message_text;
    string event_type;
    nlohmann::json original_payload; 

    bool is_valid = false; 
};

class TelegramUpdateParser {
public:
    ParsedTelegramMessage parse(const string& json_payload);
};