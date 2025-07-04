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
            callback(Json::Value()); // empty json
        } else {
            Json::Reader reader;
            Json::Value data;
            reader.parse(std::string(resp->body()), data);
            callback(data);
        }
    });
}

std::optional<Json::Value> WeatherService::fetchWeather(const std::string &city) {
    auto client = drogon::HttpClient::newHttpClient("https://api.openweathermap.org");
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/data/2.5/weather");
    req->setParameter("q", city);
    req->setParameter("appid", "<YOUR_API_KEY>"); // TODO: 
    req->setParameter("units", "metric");

    auto [result, response] = client->sendRequest(req);

    if (result != drogon::ReqResult::Ok || !response)
        return std::nullopt;

    Json::CharReaderBuilder builder;
    Json::Value root;
    std::string errs;
    const auto& body = response->body();
    auto reader = builder.newCharReader();

    if (!reader->parse(body.data(), body.data() + body.size(), &root, &errs)) {
        return std::nullopt;
    }

    return root;

    }

void WeatherService::cacheWeather(const std::string& city, const Json::Value& data) {
    // auto now = trantor::Date::now().toDbStringLocal();

    // auto db = drogon::app().getDbClient();
    // db->execSqlAsync(
    //     "INSERT INTO weather_cache (city, timestamp, json_data) VALUES ($1, $2, $3) "
    //     "ON CONFLICT (city) DO UPDATE SET timestamp = $2, json_data = $3",
    //     city,
    //     now,
    //     data.toStyledString()
    //     [](const drogon::orm::Result &) {
    //         LOG_INFO << "Weather cache updated";
    //     },
    //     [](const drogon::orm::DrogonDbException &e) {
    //         LOG_ERROR << "Weather cache update failed: " << e.base().what();
    //     }
    // );
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
