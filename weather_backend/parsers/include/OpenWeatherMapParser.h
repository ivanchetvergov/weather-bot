// OpenWeatherParser.h
#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <optional> 

struct WeatherInfo {
    std::string description;
    double temp = 0.0;
    double feels_like = 0.0;
    int humidity = 0;
    double wind_speed = 0.0;
    double pressure = 0.0; 
};

struct ForecastDayInfo {
    std::string date;
    std::string time;
    double temp = 0.0;
    double feels_like = 0.0;
    std::string description;
};


class OpenWeatherMapParser {
public:
    static std::string extractCityFromMessage(const std::string& message_text);

    static std::optional<WeatherInfo> parseCurrentWeather(const nlohmann::json& json_data);
    
    static std::string formatCurrentWeather(const std::string& city_name, const WeatherInfo& weather_info);

    static std::vector<ForecastDayInfo> parseForecast(const nlohmann::json& json_data, int days_to_show = 3);

    static std::string formatForecast(const std::string& city_name, const std::vector<ForecastDayInfo>& forecast_data);

private:
    static std::string trim(const std::string& str);
};