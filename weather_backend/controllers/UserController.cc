#include "UserController.h"
#include <drogon/orm/Mapper.h>
#include <models/Users.h>

using namespace drogon;
using namespace drogon::orm;

void UserController::getAllUsers(const HttpRequestPtr& req,
                                 std::function<void(const HttpResponsePtr&)>&& callback) {
    auto db = app().getDbClient("pg"); // read db
    Mapper<models::Users> users(db); // db controller
    // SQL call
    users.findAll( 
        // succes handler 
        [callback](const std::vector<models::Users>& allUsers) {
            Json::Value arr(Json::arrayValue);
            for (const auto& u : allUsers) {
                Json::Value item;
                item["id"] = u.getValueOfId();
                item["username"] = u.getValueOfUsername();
                item["first_name"] = u.getValueOfFirstName();
                item["created_at"] = u.getValueOfCreatedAt().toDbString();
                arr.append(item);
            }
            callback(HttpResponse::newHttpJsonResponse(arr)); // give responce 
        },
        // error handler 
        [callback](const DrogonDbException& e) {
            Json::Value err; 
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err)); // give responce 
        });
}

void UserController::createUser(const HttpRequestPtr& req,
                                std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !json->isMember("id") || !json->isMember("first_name")) {
        auto res = Json::Value();
        res["error"] = "Missing required fields: id, first_name";
        callback(HttpResponse::newHttpJsonResponse(res));
        return;
    }

    models::Users user;
    user.setId((*json)["id"].asInt64());
    user.setFirstName((*json)["first_name"].asString());

    if (json->isMember("username")) {
        user.setUsername((*json)["username"].asString());
    }

    user.setCreatedAt(trantor::Date::now());

    auto db = app().getDbClient("pg");
    Mapper<models::Users> users(db);

    users.insert(user,
        [callback](const models::Users& inserted) {
            Json::Value res;
            res["status"] = "✅ user created";
            res["id"] = inserted.getValueOfId();
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}

void UserController::deleteUser(const HttpRequestPtr& req,
                                std::function<void(const HttpResponsePtr&)>&& callback,
                                int64_t userId) {
    auto db = app().getDbClient("pg");
    Mapper<models::Users> users(db);

    users.deleteByPrimaryKey(userId,
        [callback]() {
            Json::Value res;
            res["status"] = "🗑️ user deleted";
            callback(HttpResponse::newHttpJsonResponse(res));
        },
        [callback](const DrogonDbException& e) {
            Json::Value err;
            err["error"] = e.base().what();
            callback(HttpResponse::newHttpJsonResponse(err));
        });
}
