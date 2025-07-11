// OpenWeatherParser.cc

#include "OpenWeatherMapParser.h"
#include <iomanip>   
#include <sstream>   
#include <ctime>     
#include <algorithm> 
#include <set>       

std::string OpenWeatherMapParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

std::string OpenWeatherMapParser::extractCityFromMessage(const std::string& message_text) {
    size_t first_space = message_text.find(' ');
    if (first_space != std::string::npos && first_space + 1 < message_text.length()) {
        std::string city_raw = message_text.substr(first_space + 1);
        return trim(city_raw);
    }
    return "";
}

std::optional<WeatherInfo> OpenWeatherMapParser::parseCurrentWeather(const nlohmann::json& json_data) {
    if (!json_data.contains("main") || !json_data["main"].is_object() ||
        !json_data.contains("weather") || !json_data["weather"].is_array() || json_data["weather"].empty()) {
        return std::nullopt;
    }

    WeatherInfo info;
    if (json_data["main"].contains("temp") && json_data["main"]["temp"].is_number()) {
        info.temp = json_data["main"]["temp"].get<double>();
    }
    if (json_data["main"].contains("feels_like") && json_data["main"]["feels_like"].is_number()) {
        info.feels_like = json_data["main"]["feels_like"].get<double>();
    }
    if (json_data["main"].contains("humidity") && json_data["main"]["humidity"].is_number_integer()) {
        info.humidity = json_data["main"]["humidity"].get<int>();
    }
    if (json_data.contains("wind") && json_data["wind"].is_object() && json_data["wind"].contains("speed") && json_data["wind"]["speed"].is_number()) {
        info.wind_speed = json_data["wind"]["speed"].get<double>();
    }
    if (json_data["main"].contains("pressure") && json_data["main"]["pressure"].is_number()) {
        info.pressure = json_data["main"]["pressure"].get<double>();
    }

    if (json_data["weather"][0].contains("description") && json_data["weather"][0]["description"].is_string()) {
        info.description = json_data["weather"][0]["description"].get<std::string>();
        if (!info.description.empty()) {
            info.description[0] = std::toupper(info.description[0]);
        }
    }
    return info;
}

std::string OpenWeatherMapParser::formatCurrentWeather(const std::string& city_name, const WeatherInfo& weather_info) {
    std::stringstream ss;
    ss << "Погода в городе " << city_name << ":\n"
       << weather_info.description << "\n"
       << "Температура: " << std::fixed << std::setprecision(1) << weather_info.temp << "°C (ощущается как " << std::fixed << std::setprecision(1) << weather_info.feels_like << "°C)\n"
       << "Влажность: " << weather_info.humidity << "%";
    
    
    
    return ss.str();
}

std::vector<ForecastDayInfo> OpenWeatherMapParser::parseForecast(const nlohmann::json& json_data, int days_to_show) {
    std::vector<ForecastDayInfo> forecast_list;
    if (!json_data.contains("list") || !json_data["list"].is_array()) {
        return forecast_list;
    }

    std::set<std::string> processed_dates;
    
    for (const auto& item : json_data["list"]) {
        if (forecast_list.size() >= days_to_show) break;

        if (item.contains("dt") && item["dt"].is_number_integer()) {
            time_t timestamp = item["dt"].get<time_t>();
            struct tm* local_tm = localtime(&timestamp);
            
            char date_buffer[12]; 
            strftime(date_buffer, sizeof(date_buffer), "%d.%m", local_tm);
            std::string current_date = date_buffer;

            char time_buffer[6];
            strftime(time_buffer, sizeof(time_buffer), "%H:%M", local_tm);
            std::string current_time = time_buffer;

        
        
        
            if (processed_dates.count(current_date) > 0) {
                 continue;
            }
            processed_dates.insert(current_date);


            ForecastDayInfo day_info;
            day_info.date = current_date;
            day_info.time = current_time;
            
            if (item.contains("main") && item["main"].is_object()) {
                if (item["main"].contains("temp") && item["main"]["temp"].is_number()) {
                    day_info.temp = item["main"]["temp"].get<double>();
                }
                if (item["main"].contains("feels_like") && item["main"]["feels_like"].is_number()) {
                    day_info.feels_like = item["main"]["feels_like"].get<double>();
                }
            }

            if (item.contains("weather") && item["weather"].is_array() && !item["weather"].empty()) {
                if (item["weather"][0].contains("description") && item["weather"][0]["description"].is_string()) {
                    day_info.description = item["weather"][0]["description"].get<std::string>();
                    if (!day_info.description.empty()) {
                        day_info.description[0] = std::toupper(day_info.description[0]);
                    }
                }
            }
            forecast_list.push_back(day_info);
        }
    }
    return forecast_list;
}

std::string OpenWeatherMapParser::formatForecast(const std::string& city_name, const std::vector<ForecastDayInfo>& forecast_data) {
    std::stringstream ss;
    ss << "Прогноз погоды в " << city_name << ":\n\n";

    if (forecast_data.empty()) {
        ss << "Не удалось получить прогноз на ближайшие дни.";
        return ss.str();
    }

    for (const auto& day_info : forecast_data) {
        ss << "🗓 " << day_info.date << ": ";
        ss << std::fixed << std::setprecision(1) << day_info.temp << "°C (ощущается как " << std::fixed << std::setprecision(1) << day_info.feels_like << "°C)";
        if (!day_info.description.empty()) {
            ss << ", " << day_info.description;
        }
        ss << "\n";
    }
    return ss.str();
}