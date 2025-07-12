#include "WeatherConditionParser.h"
#include <iostream>
#include <regex> // Для более сложного парсинга
#include <algorithm> // Для std::transform
#include <cctype> // Для std::tolower

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

std::optional<ParsedWeatherConditions> WeatherConditionParser::parse(const std::string& commandText, const Json::Value& nlpEntities) {
    ParsedWeatherConditions conditions;
    conditions.nlpEntities = nlpEntities; // 

    std::string lowerText = toLower(commandText); 

    auto cityOpt = extractCity(lowerText, nlpEntities);
    if (cityOpt) {
        conditions.city = *cityOpt;
    } else {

        std::regex city_regex("^/\\w+\\s+([\\p{L}\\s-]+)", std::regex::ECMAScript | std::regex::icase);
        std::smatch matches;
        if (std::regex_search(commandText, matches, city_regex) && matches.size() > 1) {
            conditions.city = matches[1].str();
        } else {
            std::cerr << "Warning: City not found in command text or NLP entities: " << commandText << std::endl;
        }
    }
    if (conditions.city.empty()) { 
        return std::nullopt; 
    }

    // 2. Извлечение условий (примеры, нужно доработать для всех полей)
    conditions.tempAbove = extractFloatCondition(lowerText, "выше", true);
    if (!conditions.tempAbove) conditions.tempAbove = extractFloatCondition(lowerText, "больше", true);
    conditions.tempBelow = extractFloatCondition(lowerText, "ниже", false);
    if (!conditions.tempBelow) conditions.tempBelow = extractFloatCondition(lowerText, "меньше", false);

    conditions.rainExpected = extractBoolCondition(lowerText, "дождь");
    if (!conditions.rainExpected) conditions.rainExpected = extractBoolCondition(lowerText, "без дождя"); // Расширить логику
    conditions.snowExpected = extractBoolCondition(lowerText, "снег");
    if (!conditions.snowExpected) conditions.snowExpected = extractBoolCondition(lowerText, "без снега");

    conditions.windSpeedGt = extractFloatCondition(lowerText, "ветер больше", true);
    conditions.windSpeedLt = extractFloatCondition(lowerText, "ветер меньше", false);
    conditions.humidityGt = extractFloatCondition(lowerText, "влажность больше", true);
    conditions.humidityLt = extractFloatCondition(lowerText, "влажность меньше", false);
    conditions.pressureGt = extractFloatCondition(lowerText, "давление больше", true);
    conditions.pressureLt = extractFloatCondition(lowerText, "давление меньше", false);

    // 3. Извлечение параметров уведомлений
    conditions.notifyTimeMinutes = extractTime(lowerText);
    conditions.notifyIntervalHours = extractInterval(lowerText);
    conditions.notifyOnChange = extractNotifyOnChange(lowerText);
    conditions.detailLevel = extractDetailLevel(lowerText);
    conditions.units = extractUnits(lowerText);

    return conditions;
}

std::optional<std::string> WeatherConditionParser::extractCity(const std::string& text, const Json::Value& nlpEntities) {
    if (nlpEntities.isObject() && nlpEntities.isMember("city") && nlpEntities["city"].isString()) {
        return nlpEntities["city"].asString();
    }
    // Здесь можно добавить более сложную логику парсинга города из текста,
    // если NLP не смог его выделить или его нет.
    // Например, можно искать известные города в тексте после команды.
    // Пока оставим это на откуп NLP или дефолтного города.
    return std::nullopt;
}

std::optional<float> WeatherConditionParser::extractFloatCondition(const std::string& text, const std::string& keyword, bool isGreaterThan) {
    // Пример: "темп выше 20", "ветер больше 10"
    std::regex re(keyword + R"(\s*(\d+(\.\d+)?))");
    std::smatch match;
    if (std::regex_search(text, match, re) && match.size() > 1) {
        try {
            return std::stof(match[1].str());
        } catch (const std::exception& e) {
            std::cerr << "Error parsing float condition: " << e.what() << std::endl;
        }
    }
    return std::nullopt;
}

std::optional<bool> WeatherConditionParser::extractBoolCondition(const std::string& text, const std::string& keyword) {
    // Пример: "если дождь", "дождь", "снег"
    if (text.find("если " + keyword) != std::string::npos || text.find(keyword) != std::string::npos) {
        return true;
    }
    // Расширить для "без дождя", "без снега"
    return std::nullopt;
}

std::optional<std::chrono::minutes> WeatherConditionParser::extractTime(const std::string& text) {
    // Пример: "в 08:00", "в 18:30"
    std::regex time_regex(R"(в\s+(\d{1,2}):(\d{2}))");
    std::smatch match;
    if (std::regex_search(text, match, time_regex) && match.size() > 2) {
        try {
            int hours = std::stoi(match[1].str());
            int minutes = std::stoi(match[2].str());
            if (hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60) {
                return std::chrono::minutes(hours * 60 + minutes);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing time: " << e.what() << std::endl;
        }
    }
    return std::nullopt;
}

std::optional<std::chrono::hours> WeatherConditionParser::extractInterval(const std::string& text) {
    // Пример: "каждые 3 часа", "раз в 6 часов"
    std::regex interval_regex(R"((?:каждые|раз в)\s+(\d+)\s+час(?:а|ов)?)");
    std::smatch match;
    if (std::regex_search(text, match, interval_regex) && match.size() > 1) {
        try {
            int hours = std::stoi(match[1].str());
            if (hours > 0) {
                return std::chrono::hours(hours);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing interval: " << e.what() << std::endl;
        }
    }
    return std::nullopt;
}

std::optional<bool> WeatherConditionParser::extractNotifyOnChange(const std::string& text) {
    // Пример: "уведомлять при изменении"
    if (text.find("при изменении") != std::string::npos || text.find("только если изменится") != std::string::npos) {
        return true;
    }
    // Добавить логику для "не уведомлять при изменении" если нужно explicit false
    return std::nullopt;
}

std::optional<std::string> WeatherConditionParser::extractDetailLevel(const std::string& text) {
    if (text.find("полная информация") != std::string::npos || text.find("подробно") != std::string::npos) {
        return "full";
    }
    if (text.find("краткая информация") != std::string::npos || text.find("кратко") != std::string::npos) {
        return "short";
    }
    return std::nullopt;
}

std::optional<std::string> WeatherConditionParser::extractUnits(const std::string& text) {
    if (text.find("имперские единицы") != std::string::npos || text.find("фаренгейт") != std::string::npos) {
        return "imperial";
    }
    if (text.find("метрические единицы") != std::string::npos || text.find("цельсий") != std::string::npos) {
        return "metric";
    }
    return std::nullopt;
}