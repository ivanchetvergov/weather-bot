// WeatherCommand.cc
#include "WeatherCommand.h"
#include "OpenWeatherMapParser.h"
#include "TelegramUpdateParser.h"
#include <iostream>
#include <iomanip> 
#include <string_view> 


using namespace std; 

WeatherCommandLogic::WeatherCommandLogic(KafkaResponseSenderPtr sender, PgDbServicePtr dbService, const string& openWeatherApiKey)
    : responseSender_(sender),
      dbService_(dbService),
      openWeatherApiKey_(openWeatherApiKey) {
    cout << "  WeatherCommandLogic initialized." << endl;
}

void WeatherCommandLogic::execute(const nlohmann::json& payload,
                                  long long telegram_user_id,
                                  const std::string& message_text,
                                  const std::string& username,
                                  const std::string& first_name
) {
    std::cout << "WeatherCommandLogic: Executing for user " << telegram_user_id << " with message: '" << message_text << "'" << std::endl;

    std::optional<std::string> city_arg_opt = TelegramUpdateParser::extractCityArgumentFromCommand(message_text);

    if (city_arg_opt.has_value() && !city_arg_opt.value().empty()) {
        std::string city = city_arg_opt.value();
        getWeatherData(city, telegram_user_id, message_text);
    } else {

        std::cerr << "ERROR: WeatherCommandLogic received message_text without a city, but it was expected." << std::endl;
        sendErrorMessage(telegram_user_id, "Не удалось определить город для погоды. Пожалуйста, укажите город. Пример: /weather Москва");
    }
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
            } 
            responseSender_->sendTelegramMessage(telegram_user_id, error_msg);
            return;
        }

        try {
            nlohmann::json data = nlohmann::json::parse(resp->body());


            string city_name_response = city; 
            if (data.contains("name") && data["name"].is_string()) {
                city_name_response = data["name"].get<string>();
            }

            std::optional<WeatherInfo> weather_info_opt = OpenWeatherMapParser::parseCurrentWeather(data);
            if (weather_info_opt) {
                std::string formatted_message = OpenWeatherMapParser::formatCurrentWeather(city_name_response, weather_info_opt.value());
                responseSender_->sendTelegramMessage(telegram_user_id, formatted_message);
            } else {
                cerr << "WeatherCommandLogic: Failed to parse weather info for city " << city << endl;
                sendErrorMessage(telegram_user_id, "Ошибка обработки данных о погоде. Попробуйте позже.");
            }

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