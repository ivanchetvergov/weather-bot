#include "WeatherCacheController.h"
#include <models/WeatherCache.h>
#include <drogon/orm/Mapper.h>

using namespace drogon;
using namespace drogon::orm;

void WeatherCacheController::get(const HttpRequestPtr& req,
                                 std::function<void(const HttpResponsePtr&)>&& callback,
                                 std::string city) {
    Mapper<models::WeatherCache> mapper(app().getDbClient("pg"));
    mapper.findByPrimaryKey(city,
        [callback](const models::WeatherCache& cache) {
            callback(HttpResponse::newHttpJsonResponse(cache.getValueOfJsonData()));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = "Cache not found";
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}

void WeatherCacheController::put(const HttpRequestPtr& req,
                                 std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !json->isMember("city") || !json->isMember("data")) {
        Json::Value err;
        err["error"] = "Missing fields: city, data";
        callback(HttpResponse::newHttpJsonResponse(err));
        return;
    }

    models::WeatherCache cache;
    cache.setCity((*json)["city"].asString());
    cache.setTimestamp(trantor::Date::now());
    cache.setJsonData((*json)["data"]);

    Mapper<models::WeatherCache> mapper(app().getDbClient("pg"));
    mapper.insert(cache,
        [callback](const models::WeatherCache&) {
            Json::Value res;
            res["status"] = "cache updated";
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}
