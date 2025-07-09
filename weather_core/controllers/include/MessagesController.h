#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class MessagesController : public HttpController<MessagesController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(MessagesController::getAll, "/messages", Get);
        ADD_METHOD_TO(MessagesController::getOne, "/messages/{1}", Get);
        ADD_METHOD_TO(MessagesController::create, "/messages", Post);
        ADD_METHOD_TO(MessagesController::remove, "/messages/{1}", Delete);
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
