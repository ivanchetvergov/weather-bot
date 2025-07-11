// ForecastCommand.cc

#include "ForecastCommand.h"
#include "OpenWeatherMapParser.h"
#include "TelegramUpdateParser.h"
#include <iostream>
#include <string_view>
#include <sstream>

using namespace std;

ForecastCommandLogic::ForecastCommandLogic(KafkaResponseSenderPtr sender, PgDbServicePtr dbService, const string& openWeatherApiKey)
    : responseSender_(sender),
      dbService_(dbService),
      openWeatherApiKey_(openWeatherApiKey) {
    cout << "  ForecastCommandLogic initialized." << endl;
}

void ForecastCommandLogic::execute(const nlohmann::json& payload,
                                   long long telegram_user_id,
                                   const std::string& message_text, 
                                   const std::string& username,
                                   const std::string& first_name
) {
    std::cout << "ForecastCommandLogic: Executing for user " << telegram_user_id << " with message: '" << message_text << "'" << std::endl;

    std::optional<std::string> city_arg_opt = TelegramUpdateParser::extractCityArgumentFromCommand(message_text);

    if (city_arg_opt.has_value() && !city_arg_opt.value().empty()) {
        std::string city = city_arg_opt.value();
        getForecastData(city, telegram_user_id, message_text);
    } else {
        std::cerr << "ERROR: ForecastCommandLogic received message_text without a city, but it was expected." << std::endl;
        sendErrorMessage(telegram_user_id, "Не удалось определить город для прогноза. Пожалуйста, укажите город. Пример: /forecast Москва");
    }
}

void ForecastCommandLogic::getForecastData(
    const string& city,
    long long telegram_user_id,
    const string& original_message_text
) {
    auto client = drogon::HttpClient::newHttpClient("https://api.openweathermap.org");
    drogon::HttpRequestPtr req = drogon::HttpRequest::newHttpRequest();

    std::string path = "/data/2.5/forecast";
    std::string query_params = "?q=" + city + "&appid=" + openWeatherApiKey_ + "&units=metric&lang=ru";
    req->setPath(path + query_params);

    client->sendRequest(req, [=, this](drogon::ReqResult result, const drogon::HttpResponsePtr &resp) {
        if (result != drogon::ReqResult::Ok) {
            cerr << "ForecastCommandLogic: HTTP request failed for city " << city << ": " << endl;
            sendErrorMessage(telegram_user_id, "Извините, не удалось подключиться к сервису погоды. Попробуйте позже.");
            return;
        }

        if (resp->statusCode() != drogon::k200OK) {
            cerr << "ForecastCommandLogic: OpenWeatherMap API returned status " << resp->statusCode() << " for city " << city << endl;
            string error_msg = "Извините, не удалось получить данные о прогнозе для города '" + city + "'. Проверьте название города.";
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
            if (data.contains("city") && data["city"].contains("name") && data["city"]["name"].is_string()) {
                city_name_response = data["city"]["name"].get<string>();
            }
            
            std::vector<ForecastDayInfo> forecast_data = OpenWeatherMapParser::parseForecast(data, 3);
            std::string formatted_message = OpenWeatherMapParser::formatForecast(city_name_response, forecast_data);
            
            responseSender_->sendTelegramMessage(telegram_user_id, formatted_message);

        } catch (const nlohmann::json::parse_error& e) {
            cerr << "ForecastCommandLogic: JSON parse error from OpenWeatherMap for city " << city << ": " << e.what() << endl;
            sendErrorMessage(telegram_user_id, "Ошибка обработки данных прогноза. Попробуйте позже.");
        } catch (const nlohmann::json::exception& e) {
            cerr << "ForecastCommandLogic: JSON access error from OpenWeatherMap for city " << city << ": " << e.what() << endl;
            sendErrorMessage(telegram_user_id, "Ошибка обработки данных прогноза. Попробуйте позже.");
        } catch (const exception& e) {
            cerr << "ForecastCommandLogic: Unexpected error during forecast processing for city " << city << ": " << e.what() << endl;
            sendErrorMessage(telegram_user_id, "Произошла непредвиденная ошибка при получении прогноза.");
        }
    });
}

void ForecastCommandLogic::sendErrorMessage(long long telegram_user_id, const string& error_message) {
    if (responseSender_) {
        responseSender_->sendTelegramMessage(telegram_user_id, error_message);
    } else {
        cerr << "ERROR: responseSender_ is null in ForecastCommandLogic. Cannot send error message to " << telegram_user_id << ": " << error_message << endl;
    }
}