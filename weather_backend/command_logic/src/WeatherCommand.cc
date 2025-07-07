#include "WeatherCommand.h"
#include <iostream>
#include <iomanip> 
#include <string_view> 


using namespace std; 

WeatherCommandLogic::WeatherCommandLogic(KafkaResponseSenderPtr sender, const string& openWeatherApiKey)
    : responseSender_(sender),
      openWeatherApiKey_(openWeatherApiKey) {
    cout << "  WeatherCommandLogic initialized." << endl;
}

void WeatherCommandLogic::execute(
    PgDbServicePtr db_service,
    const nlohmann::json& payload,
    long long telegram_user_id,
    const string& message_text, 
    const string& username,
    const string& first_name
) {
    cout << "WeatherCommandLogic: Executing for user " << telegram_user_id << " with message: " << message_text << endl;

    if (db_service) {
        MessageData msg_data;
        msg_data.user_id = telegram_user_id;
        msg_data.text = message_text;
        db_service->insertMessage(msg_data); 
    } else {
        cerr << "WARNING: PgDbService is null in WeatherCommandLogic. Cannot save message to DB." << endl;
    }

    size_t first_space = message_text.find(' ');
    string city;
    if (first_space != string::npos && first_space + 1 < message_text.length()) {
        city = message_text.substr(first_space + 1);
        size_t start = city.find_first_not_of(" \t\n\r\f\v");
        size_t end = city.find_last_not_of(" \t\n\r\f\v");
        if (string::npos == start) {
            city = "";
        } else {
            city = city.substr(start, end - start + 1);
        }
    }

    if (city.empty()) {
        responseSender_->sendTelegramMessage(telegram_user_id,
                                             "Пожалуйста, укажите город. Пример: /weather Москва");
        return;
    }

    getWeatherData(city, telegram_user_id, message_text);
}

void WeatherCommandLogic::getWeatherData(
    const string& city,
    long long telegram_user_id,
    const string& original_message_text
) {

    auto client = drogon::HttpClient::newHttpClient("https://api.openweathermap.org");
    drogon::HttpRequestPtr req = drogon::HttpRequest::newHttpRequest();


    std::string path = "/data/2.5/weather";
    std::string query_params = "?q=" + city + "&appid=" + openWeatherApiKey_ + "&units=metric&lang=ru";
    req->setPath(path + query_params);

    client->sendRequest(req, [=, this](drogon::ReqResult result, const drogon::HttpResponsePtr &resp) {
        if (result != drogon::ReqResult::Ok) {
            cerr << "WeatherCommandLogic: HTTP request failed for city " << city << ": " << endl;
            sendErrorMessage(telegram_user_id, "Извините, не удалось подключиться к сервису погоды. Попробуйте позже.");
            return;
        }

        if (resp->statusCode() != drogon::k200OK) {
            cerr << "WeatherCommandLogic: OpenWeatherMap API returned status " << resp->statusCode() << " for city " << city << endl;
            string error_msg = "Извините, не удалось получить данные о погоде для города '" + city + "'. Проверьте название города.";
            if (resp->statusCode() == drogon::k401Unauthorized) {
                error_msg = "Ошибка авторизации с OpenWeatherMap API. Проверьте ключ API.";
                cerr << "OpenWeatherMap API key might be invalid!" << endl;
            } else if (resp->statusCode() == drogon::k404NotFound) {
                // Это уже обрабатывается в сообщении выше
            }
            responseSender_->sendTelegramMessage(telegram_user_id, error_msg);
            return;
        }

        try {
            nlohmann::json data = nlohmann::json::parse(resp->body());

            string weather_desc = "Неизвестно";
            double temp = 0.0;
            double feels_like = 0.0;
            int humidity = 0;
            string city_name = city; 

            if (data.contains("weather") && data["weather"].is_array() && !data["weather"].empty()) {
                if (data["weather"][0].contains("description") && data["weather"][0]["description"].is_string()) {
                    weather_desc = data["weather"][0]["description"].get<string>();
                    // Убедитесь, что первая буква заглавная
                    if (!weather_desc.empty()) {
                        weather_desc[0] = toupper(weather_desc[0]);
                    }
                }
            }
            if (data.contains("main") && data["main"].is_object()) {
                if (data["main"].contains("temp") && data["main"]["temp"].is_number()) {
                    temp = data["main"]["temp"].get<double>();
                }
                if (data["main"].contains("feels_like") && data["main"]["feels_like"].is_number()) {
                    feels_like = data["main"]["feels_like"].get<double>();
                }
                if (data["main"].contains("humidity") && data["main"]["humidity"].is_number_integer()) {
                    humidity = data["main"]["humidity"].get<int>();
                }
            }
            if (data.contains("name") && data["name"].is_string()) {
                city_name = data["name"].get<string>();
            }

            stringstream ss;
            ss << "Погода в городе " << city_name << ":\n"
               << weather_desc << "\n"
               << "Температура: " << fixed << setprecision(1) << temp << "°C (ощущается как " << fixed << setprecision(1) << feels_like << "°C)\n"
               << "Влажность: " << humidity << "%";

            responseSender_->sendTelegramMessage(telegram_user_id, ss.str());

        } catch (const nlohmann::json::parse_error& e) {
            cerr << "WeatherCommandLogic: JSON parse error from OpenWeatherMap for city " << city << ": " << e.what() << endl;
            sendErrorMessage(telegram_user_id, "Ошибка обработки данных о погоде. Попробуйте позже.");
        } catch (const nlohmann::json::exception& e) {
            cerr << "WeatherCommandLogic: JSON access error from OpenWeatherMap for city " << city << ": " << e.what() << endl;
            sendErrorMessage(telegram_user_id, "Ошибка обработки данных о погоде. Попробуйте позже.");
        } catch (const exception& e) {
            cerr << "WeatherCommandLogic: Unexpected error during weather processing for city " << city << ": " << e.what() << endl;
            sendErrorMessage(telegram_user_id, "Произошла непредвиденная ошибка при получении погоды.");
        }
    });
}

void WeatherCommandLogic::sendErrorMessage(long long telegram_user_id, const string& error_message) {
    if (responseSender_) {
        responseSender_->sendTelegramMessage(telegram_user_id, error_message);
    } else {
        cerr << "ERROR: responseSender_ is null in WeatherCommandLogic. Cannot send error message to " << telegram_user_id << ": " << error_message << endl;
    }
}