#include "../include/TelegramController.h"
#include <drogon/orm/Mapper.h>
#include "../models/Users.h"  

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::weather_bot;

void Telegram::handleWebhook(const HttpRequestPtr& req,
                             std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json).isMember("message")) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setBody("No message");
        callback(resp);
        return;
    }

    auto from = (*json)["message"]["from"];
    int64_t id = from["id"].asInt64();
    std::string firstName = from["first_name"].asString();
    std::string username = from.isMember("username") ? from["username"].asString() : "";

    // save user to db
    auto client = app().getDbClient("pg");
    Mapper<Users> users(client);

    Users user;
    user.setId(id);
    user.setFirstName(firstName);
    user.setUsername(username);
    user.setCreatedAt(trantor::Date::now());

    users.insert(user,
        [callback](const Users &u) {
            Json::Value res;
            res["status"] = "saved";
            res["id"] = u.getValueOfId();
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException &e) {
            Json::Value res;
            res["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(res));
        });
}
