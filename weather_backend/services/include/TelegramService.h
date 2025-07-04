#pragma once

#include <drogon/HttpClient.h>
#include <json/json.h>
#include <string>
#include <optional>
#include <functional>

struct TelegramCommand {
    std::string intent;  // "get_weather", "unsubscribe", etc.
    std::map<std::string, std::string> entities; // city: "Moscow", etc.
};

class TelegramService {
public:
    static void sendMessage(int64_t chatId, const std::string& text);

    static void sendMessageWithButtons(int64_t chatId,
                                       const std::string& text,
                                       const Json::Value& buttons);

    static void logEvent(int64_t userId, const std::string& type, const std::string& data);

    static void handleIncomingUpdate(const Json::Value& update);

    static std::optional<std::string> extractText(const Json::Value& update);

    /// call NLP for interpret text
    static std::optional<TelegramCommand> interpretCommand(const std::string& userText);

    /// make command after NLP analys
    static void processCommand(int64_t chatId, const TelegramCommand& command);
};
