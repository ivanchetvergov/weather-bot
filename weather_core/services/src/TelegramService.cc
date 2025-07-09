// TelegramService.cpp
#include "TelegramService.h"
#include <drogon/HttpAppFramework.h>
#include <json/json.h>

void TelegramService::sendMessage(int64_t chatId, const std::string& text) {
    static const std::string botToken = "your_bot_token_here"; // TODO: вынести в конфиг
    auto client = drogon::HttpClient::newHttpClient("https://api.telegram.org");

    Json::Value json;
    json["chat_id"] = std::to_string(chatId);
    json["text"] = text;

    auto req = drogon::HttpRequest::newHttpJsonRequest(json);
    req->setPath("/bot" + botToken + "/sendMessage");

    client->sendRequest(req, [](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
        if (result != drogon::ReqResult::Ok) {
            LOG_ERROR << "telegram error: " << result;
        } else {
            LOG_INFO << "message sent to telegram";
        }
    });
}
