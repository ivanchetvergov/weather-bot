#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class UserController : public drogon::HttpController<UserController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(UserController::getAllUsers, "/users", Get);
        ADD_METHOD_TO(UserController::createUser, "/users", Post);
        ADD_METHOD_TO(UserController::deleteUser, "/users/{1}", Delete);
    METHOD_LIST_END

    void getAllUsers(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback);

    void createUser(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback);

    void deleteUser(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback,
                    int64_t userId);
};
