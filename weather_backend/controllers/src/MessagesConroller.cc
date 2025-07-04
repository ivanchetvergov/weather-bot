#include "../include/MessagesController.h"
#include <models/Messages.h>
#include <drogon/orm/Mapper.h>

using namespace drogon;
using namespace drogon::orm;

void MessagesController::getAll(const HttpRequestPtr& req,
                                std::function<void(const HttpResponsePtr&)>&& callback) {
    Mapper<models::Messages> mapper(app().getDbClient("pg")); 
    // SQL call
    mapper.findAll(
        [callback](const std::vector<models::Messages>& messages) {
            Json::Value arr(Json::arrayValue);
            for (const auto& m : messages) {
                Json::Value item;
                item["id"] = m.getValueOfId();
                item["user_id"] = m.getValueOfUserId();
                item["text"] = m.getValueOfText();
                item["created_at"] = m.getValueOfCreatedAt().toDbString();
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

void MessagesController::getOne(const HttpRequestPtr& req,
                                std::function<void(const HttpResponsePtr&)>&& callback,
                                int id) {
    Mapper<models::Messages> mapper(app().getDbClient("pg"));
    mapper.findByPrimaryKey(id,
        // if found id send HTTP response
        [callback](const models::Messages& m) {
            Json::Value res;
            res["id"] = m.getValueOfId();
            res["user_id"] = m.getValueOfUserId();
            res["text"] = m.getValueOfText();
            res["created_at"] = m.getValueOfCreatedAt().toDbString();
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}

void MessagesController::create(const HttpRequestPtr& req,
                                std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !json->isMember("user_id") || !json->isMember("text")) {
        Json::Value err;
        err["error"] = "Missing required fields: user_id, text";
        callback(HttpResponse::newHttpJsonResponse(err));
        return;
    }

    models::Messages msg;
    msg.setUserId((*json)["user_id"].asInt64());
    msg.setText((*json)["text"].asString());
    msg.setCreatedAt(trantor::Date::now());

    Mapper<models::Messages> mapper(app().getDbClient("pg"));
    mapper.insert(msg,
        [callback](const models::Messages& inserted) {
            Json::Value res;
            res["status"] = "message saved";
            res["id"] = inserted.getValueOfId();
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}

void MessagesController::remove(const HttpRequestPtr& req,
                                std::function<void(const HttpResponsePtr&)>&& callback,
                                int id) {
    Mapper<models::Messages> mapper(app().getDbClient("pg"));
    mapper.deleteByPrimaryKey(id,
        [callback]() {
            Json::Value res;
            res["status"] = "message deleted";
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}
