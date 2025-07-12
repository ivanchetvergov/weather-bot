#pragma once

#include <nlohmann/json.hpp> // for json parsing
#include <string>           
#include <iostream> 
#include <optional> 

#include "DataTransferObjects.h" // ParsedTelegramMessage

class TelegramUpdateParser {
public:
    // * @brief parses a json payload string (expected from kafka) into a ParsedTelegramMessage object.
    ParsedTelegramMessage parse(const std::string& json_payload);

    // * @brief extracts the base command (e.g., "/weather" from "/weather london").
    std::string extractBaseCommand(const std::string& full_command_text);
    
    // * @brief extracts the city argument from a command text (e.g., "london" from "/weather london").
    static std::optional<std::string> extractCityArgumentFromCommand(const std::string& full_command_text);
};