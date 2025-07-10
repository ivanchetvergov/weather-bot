#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <iostream> 

#include "DataTransferObjects.h"


class TelegramUpdateParser {
public:
    ParsedTelegramMessage parse(const std::string& json_payload);
    std::string extractBaseCommand(const std::string& full_command_text);
};