#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class AlertsController : public HttpController<AlertsController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(AlertsController::getAll, "/alerts", Get);
        ADD_METHOD_TO(AlertsController::getOne, "/alerts/{1}", Get);
        ADD_METHOD_TO(AlertsController::create, "/alerts", Post);
        ADD_METHOD_TO(AlertsController::remove, "/alerts/{1}", Delete);
    METHOD_LIST_END

    void getAll(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback);

    void getOne(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback,
                int id);

    void create(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback);

    void remove(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback,
                int id);
};
