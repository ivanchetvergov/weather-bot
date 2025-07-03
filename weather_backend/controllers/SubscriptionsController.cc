#include "SubscriptionsController.h"
#include <models/Subscriptions.h>
#include <drogon/orm/Mapper.h>

using namespace drogon;
using namespace drogon::orm;

void SubscriptionsController::getAll(const HttpRequestPtr& req,
                                     std::function<void(const HttpResponsePtr&)>&& callback) {
    Mapper<models::Subscriptions> mapper(app().getDbClient("pg"));
    mapper.findAll(
        [callback](const std::vector<models::Subscriptions>& subs) {
            Json::Value arr(Json::arrayValue);
            for (const auto& s : subs) {
                Json::Value item;
                item["id"] = s.getValueOfId();
                item["user_id"] = s.getValueOfUserId();
                item["city"] = s.getValueOfCity();
                item["temp_above"] = s.getValueOfTempAbove();
                item["rain"] = s.getValueOfRain();
                item["wind_speed_gt"] = s.getValueOfWindSpeedGt();
                item["notify_time"] = s.getValueOfNotifyTime().toFormattedString(false);
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

    models::Subscriptions sub;
    sub.setUserId((*json)["user_id"].asInt64());
    sub.setCity((*json)["city"].asString());

    if (json->isMember("temp_above")) sub.setTempAbove((*json)["temp_above"].asFloat());
    if (json->isMember("rain")) sub.setRain((*json)["rain"].asBool());
    if (json->isMember("wind_speed_gt")) sub.setWindSpeedGt((*json)["wind_speed_gt"].asFloat());
    if (json->isMember("notify_time")) sub.setNotifyTime(trantor::Date::fromDbStringLocal((*json)["notify_time"].asString()));

    Mapper<models::Subscriptions> mapper(app().getDbClient("pg"));
    mapper.insert(sub,
        [callback](const models::Subscriptions& inserted) {
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
    Mapper<models::Subscriptions> mapper(app().getDbClient("pg"));
    mapper.deleteByPrimaryKey(id,
        [callback]() {
            Json::Value res;
            res["status"] = "🗑️ subscription deleted";
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}
