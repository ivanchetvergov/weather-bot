#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <iostream> 

#include "DataTransferObjects.h"

using namespace string;

class TelegramUpdateParser {
public:
    ParsedTelegramMessage parse(const string& json_payload);
};