#pragma once
#include <drogon/HttpClient.h>
#include <json/json.h>
#include <string>
#include <optional>
#include "models/WeatherCache.h"

class WeatherService {
public:
    static std::optional<Json::Value> fetchWeather(const std::string& city);
    static void cacheWeather(const std::string& city, const Json::Value& data);
    static std::optional<Json::Value> getCachedWeather(const std::string& city);

    static std::optional<float> extractTemperature(const Json::Value& data);
    static std::optional<float> extractWindSpeed(const Json::Value& data);
    static std::optional<bool> extractRain(const Json::Value& data);
};
