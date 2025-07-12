#pragma once

#include <drogon/orm/Mapper.h>
#include <json/json.h>
#include <string>
#include <optional>
#include <chrono> 

#include "DataTransferObjects.h" 

class WeatherConditionParser {
public:
    static std::optional<ParsedWeatherConditions> parse(const std::string& commandText, const Json::Value& nlpEntities = Json::nullValue);

private:
    static std::optional<std::string> extractCity(const std::string& text, 
                                                const Json::Value& nlpEntities);
    static std::optional<float> extractFloatCondition(const std::string& text, 
                                const std::string& keyword, bool isGreaterThan);
    static std::optional<bool> extractBoolCondition(const std::string& text, 
                                                    const std::string& keyword);
    static std::optional<std::chrono::minutes> extractTime(const std::string& text);

    static std::optional<std::chrono::hours> extractInterval(const std::string& text);

    static std::optional<bool> extractNotifyOnChange(const std::string& text);

    static std::optional<std::string> extractDetailLevel(const std::string& text);

    static std::optional<std::string> extractUnits(const std::string& text);

};