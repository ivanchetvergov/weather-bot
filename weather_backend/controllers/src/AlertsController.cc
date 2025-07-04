#include <AlertsController.h>
#include <models/Alerts.h>
#include <drogon/orm/Mapper.h>

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::weather_bot;

void AlertsController::getAll(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& callback) {
    Mapper<Alerts> mapper(app().getDbClient("pg"));
    mapper.findAll(
        [callback](const std::vector<Alerts>& alerts) {
            Json::Value arr(Json::arrayValue);
            for (const auto& a : alerts) {
                Json::Value item;
                item["id"] = a.getValueOfId();
                item["user_id"] = a.getValueOfUserId();
                item["city"] = a.getValueOfCity();
                item["condition"] = a.getValueOfCondition();
                item["sent_at"] = a.getValueOfSentAt().toDbString();
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

void AlertsController::getOne(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& callback,
                              int id) {
    Mapper<Alerts> mapper(app().getDbClient("pg"));
    mapper.findByPrimaryKey(id,
        [callback](const Alerts& a) {
            Json::Value res;
            res["id"] = a.getValueOfId();
            res["user_id"] = a.getValueOfUserId();
            res["city"] = a.getValueOfCity();
            res["condition"] = a.getValueOfCondition();
            res["sent_at"] = a.getValueOfSentAt().toDbString();
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}

void AlertsController::create(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !json->isMember("user_id") || !json->isMember("city") || !json->isMember("condition")) {
        Json::Value err;
        err["error"] = "Missing required fields: user_id, city, condition";
        callback(HttpResponse::newHttpJsonResponse(err));
        return;
    }

    Alerts alert;
    alert.setUserId((*json)["user_id"].asInt64());
    alert.setCity((*json)["city"].asString());
    alert.setCondition((*json)["condition"].asString());
    alert.setSentAt(trantor::Date::now());

    Mapper<Alerts> mapper(app().getDbClient("pg"));
    mapper.insert(alert,
        [callback](const Alerts& inserted) {
            Json::Value res;
            res["status"] = "alert created";
            res["id"] = inserted.getValueOfId();
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}

void AlertsController::remove(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& callback,
                              int id) {
    Mapper<Alerts> mapper(app().getDbClient("pg"));
    mapper.deleteByPrimaryKey(id,
    [callback](size_t count) {
        Json::Value res;
        res["status"] = count > 0 ? "alert deleted" : "not found";
        callback(HttpResponse::newHttpJsonResponse(res));
    },
    [callback](const DrogonDbException& e) {
        Json::Value err;
        err["error"] = e.base().what();
        callback(HttpResponse::newHttpJsonResponse(err));
    });

}
