#include "WeatherService.h"
#include <drogon/orm/DbClient.h>
#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

void WeatherService::fetchWeather(const std::string& city,
                                  std::function<void(const Json::Value&)> callback) {
    static const std::string apiKey = "your_api_key_here"; // TODO: добавить в конфиг
    auto client = drogon::HttpClient::newHttpClient("https://api.openweathermap.org");

    std::string path = "/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=metric";

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath(path);

    client->sendRequest(req, [callback](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
        if (result != drogon::ReqResult::Ok) {
            LOG_ERROR << "weather api error: " << result;
            callback(Json::Value()); // пустой json
        } else {
            Json::Reader reader;
            Json::Value data;
            reader.parse(resp->body(), data);
            callback(data);
        }
    });
}


void WeatherService::cacheWeather(const std::string& city, const Json::Value& data) {
    auto db = app().getDbClient();
    auto now = trantor::Date::now().toDbStringLocal();

    db->execSqlAsync(
        "INSERT INTO weather_cache (city, timestamp, json_data) VALUES ($1, $2, $3) "
        "ON CONFLICT (city) DO UPDATE SET timestamp = $2, json_data = $3",
        city, now, data.toStyledString());
}

std::optional<Json::Value> WeatherService::getCachedWeather(const std::string& city) {
    auto db = app().getDbClient();
    auto result = db->execSqlSync("SELECT json_data FROM weather_cache WHERE city = $1", city);

    if (!result.empty()) {
        Json::Value json;
        Json::Reader reader;
        reader.parse(result[0]["json_data"].as<std::string>(), json);
        return json;
    }
    return std::nullopt;
}

std::optional<float> WeatherService::extractTemperature(const Json::Value& data) {
    if (data.isMember("temp"))
        return data["temp"].asFloat();
    return std::nullopt;
}

std::optional<float> WeatherService::extractWindSpeed(const Json::Value& data) {
    if (data.isMember("wind_speed"))
        return data["wind_speed"].asFloat();
    return std::nullopt;
}

std::optional<bool> WeatherService::extractRain(const Json::Value& data) {
    if (data.isMember("rain"))
        return data["rain"].asBool();
    return std::nullopt;
}
