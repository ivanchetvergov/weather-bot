#include "../include/SubscriptionsController.h"
#include <models/Subscriptions.h>
#include <drogon/orm/Mapper.h>

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::weather_bot;

void SubscriptionsController::getAll(const HttpRequestPtr& req,
                                     std::function<void(const HttpResponsePtr&)>&& callback) {
    Mapper<Subscriptions> mapper(app().getDbClient("pg"));
    mapper.findAll(
        [callback](const std::vector<Subscriptions>& subs) {
            Json::Value arr(Json::arrayValue);
            for (const auto& s : subs) {
                Json::Value item;
                item["id"] = s.getValueOfId();
                item["user_id"] = s.getValueOfUserId();
                item["city"] = s.getValueOfCity();
                item["temp_above"] = s.getValueOfTempAbove();
                item["rain"] = s.getValueOfRain();
                item["wind_speed_gt"] = s.getValueOfWindSpeedGt();
                item["notify_time"] = s.getValueOfNotifyTime();
                arr.append(item);
            }
            callback(HttpResponse::newHttpJsonResponse(arr));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}

void SubscriptionsController::create(const HttpRequestPtr& req,
                                     std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !json->isMember("user_id") || !json->isMember("city")) {
        Json::Value err;
        err["error"] = "Missing required fields: user_id, city";
        callback(HttpResponse::newHttpJsonResponse(err));
        return;
    }

    Subscriptions sub;
    sub.setUserId((*json)["user_id"].asInt64());
    sub.setCity((*json)["city"].asString());

    if (json->isMember("temp_above")) sub.setTempAbove((*json)["temp_above"].asFloat());
    if (json->isMember("rain")) sub.setRain((*json)["rain"].asBool());
    if (json->isMember("wind_speed_gt")) sub.setWindSpeedGt((*json)["wind_speed_gt"].asFloat());
    if (json->isMember("notify_time")) sub.setNotifyTime((*json)["notify_time"].asString());

    Mapper<Subscriptions> mapper(app().getDbClient("pg"));
    mapper.insert(sub,
        [callback](const Subscriptions& inserted) {
            Json::Value res;
            res["status"] = "subscription saved";
            res["id"] = inserted.getValueOfId();
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}

void SubscriptionsController::remove(const HttpRequestPtr& req,
                                     std::function<void(const HttpResponsePtr&)>&& callback,
                                     int id) {
    Mapper<Subscriptions> mapper(app().getDbClient("pg"));
    mapper.deleteByPrimaryKey(id,
        [callback](size_t count) {
            Json::Value res;
            res["status"] = (count > 0) ? "subscription deleted" : "not found";
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}
