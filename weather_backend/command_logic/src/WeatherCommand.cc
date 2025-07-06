#include "WeatherCommandLogic.h"
#include <iostream>
#include <iomanip> 
#include <string_view> /


using namespace std; 

WeatherCommandLogic::WeatherCommandLogic(KafkaResponseSenderPtr sender, const string& openWeatherApiKey)
    : responseSender_(sender),
      openWeatherApiKey_(openWeatherApiKey) {
    cout << "  WeatherCommandLogic initialized." << endl;
}

void WeatherCommandLogic::execute(
    drogon::orm::DbClientPtr dbClient, 
    const nlohmann::json& payload,
    long long telegram_user_id,
    const string& message_text, 
    const string& username,
    const string& first_name
) {
    cout << "WeatherCommandLogic: Executing for user " << telegram_user_id << " with message: " << message_text << endl;

    size_t first_space = message_text.find(' ');
    string city;
    if (first_space != string::npos && first_space + 1 < message_text.length()) {
        city = message_text.substr(first_space + 1); // Получаем все, что после "/weather "
        // Удаляем лишние пробелы в начале/конце, если они есть (простая обрезка)
        size_t start = city.find_first_not_of(" \t\n\r\f\v");
        size_t end = city.find_last_not_of(" \t\n\r\f\v");
        if (string::npos == start) {
            city = ""; // Только пробелы
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

void WeatherCommandLogic::saveMessageToDb(
    drogon::orm::DbClientPtr dbClient,
    long long telegram_user_id,
    const std::string& message_text
) {
    Mapper<drogon_model::weather_backend::Messages> messageMapper(dbClient);
    drogon_model::weather_backend::Messages newMessage;
    newMessage.setUserId(telegram_user_id);
    newMessage.setText(message_text);

    messageMapper.insert(newMessage,
        [=](drogon_model::weather_backend::Messages insertedMessage) {
            cout << "Message for user " << telegram_user_id << " saved to DB." << endl;
        },
        [=](const DrogonDbException& e) {
            cerr << "Error saving message for user " << telegram_user_id << ": " << e.what() << endl;
        });
}

void WeatherCommandLogic::getWeatherData(
    const string& city,
    long long telegram_user_id,
    const string& original_message_text
) {
    // Создаем HTTP клиент
    auto client = drogon::HttpClient::newHttpClient("https://api.openweathermap.org");
    drogon::HttpRequestPtr req = drogon::HttpRequest::newHttpRequest();

    // Формируем URL для запроса
    // Используем std::string для параметров, чтобы избежать проблем с C-строками и URL-кодированием
    std::string path = "/data/2.5/weather";
    std::string query_params = "?q=" + city + "&appid=" + openWeatherApiKey_ + "&units=metric&lang=ru";
    req->setPath(path + query_params);

    // Отправляем GET запрос
    client->sendRequest(req, [=](drogon::ReqResult result, const drogon::HttpResponsePtr &resp) {
        if (result != drogon::ReqResult::Ok) {
            cerr << "WeatherCommandLogic: HTTP request failed for city " << city << ": " << drogon::to</td>::string(result) << endl;
            sendErrorMessage(telegram_user_id, "Извините, не удалось подключиться к сервису погоды. Попробуйте позже.");
            return;
        }

        if (resp->statusCode() != drogon::k200Ok) {
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
            string city_name = city; // По умолчанию, если нет в ответе

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