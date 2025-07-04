// NotificationService.cpp
#include "NotificationService.h"
#include "TelegramService.h"
#include "WeatherService.h"
#include <drogon/orm/DbClient.h>
#include <drogon/HttpAppFramework.h>

using namespace drogon::orm;

void NotificationService::checkAndNotify() {
    auto db = drogon::app().getDbClient();

    // get all subs
    db->execSqlAsync("SELECT s.user_id, s.city, s.temp_above, s.rain, s.wind_speed_gt, u.id as chat_id 
        FROM subscriptions s JOIN users u ON s.user_id = u.id",
        [](const Result& res) {
            for (const auto& row : res) {
                int64_t userId = row["user_id"].as<int64_t>();
                std::string city = row["city"].as<std::string>();
                float tempAbove = row["temp_above"].isNull() ? -1000.0f : row["temp_above"].as<float>();
                bool wantsRain = !row["rain"].isNull() && row["rain"].as<bool>();
                float windSpeedGt = row["wind_speed_gt"].isNull() ? -1.0f : row["wind_speed_gt"].as<float>();
                int64_t chatId = row["chat_id"].as<int64_t>();

                WeatherService::fetchWeather(city, [=](const Json::Value& weather) {
                    if (!weather.isObject()) return;

                    float temp = weather["main"]["temp"].asFloat();
                    bool rain = weather.isMember("rain");
                    float wind = weather["wind"]["speed"].asFloat();

                    bool notify = false;
                    std::string reason;

                    if (temp > tempAbove) {
                        notify = true;
                        reason += "температура выше " + std::to_string(tempAbove) + "°C\n";
                    }
                    if (wantsRain && rain) {
                        notify = true;
                        reason += "идёт дождь\n";
                    }
                    if (wind > windSpeedGt) {
                        notify = true;
                        reason += "ветер сильнее " + std::to_string(windSpeedGt) + " м/с\n";
                    }

                    if (notify) {
                        std::string text = "в городе " + city + " сейчас:\n" + reason;
                        TelegramService::sendMessage(chatId, text);
                    }
                });
            }
        },
        [](const DrogonDbException& e) {
            LOG_ERROR << "db error: " << e.base().what();
        }
    );
}
