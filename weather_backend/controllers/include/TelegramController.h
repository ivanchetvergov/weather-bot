#pragma once

#include <drogon/HttpController.h>  
#include <drogon/HttpRequest.h>     
#include <drogon/HttpResponse.h>   
#include <functional>     

using namespace drogon;

class Telegram : public HttpController<Telegram> {
  public:
    METHOD_LIST_BEGIN
      ADD_METHOD_TO(Telegram::handleWebhook, "/tg", Post);
    METHOD_LIST_END

    void handleWebhook(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback);
};
