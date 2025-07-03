#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class SubscriptionsController : public HttpController<SubscriptionsController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(SubscriptionsController::getAll, "/subscriptions", Get);
        ADD_METHOD_TO(SubscriptionsController::create, "/subscriptions", Post);
        ADD_METHOD_TO(SubscriptionsController::remove, "/subscriptions/{1}", Delete);
    METHOD_LIST_END

    void getAll(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback);

    void create(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback);

    void remove(const HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                int id);
};
