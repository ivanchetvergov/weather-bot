#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp> 
#include "IResponseSender.h" 

class ICommandLogic {
public:
    virtual ~ICommandLogic() = default;

    // Метод для выполнения логики команды
    // Принимает:
    // - payload: полный JSON-объект Kafka-сообщения (для гибкости)
    // - telegram_user_id: ID пользователя (общий для всех Telegram-команд)
    // - message_text: текст сообщения (общий для всех Telegram-команд)
    // - username, first_name: данные пользователя (если есть, могут быть пустыми)
    virtual void execute(const nlohmann::json& payload,
                         long long telegram_user_id,
                         const std::string& message_text,
                         const std::string& username,
                         const std::string& first_name) = 0;
};

using ICommandLogicPtr = std::shared_ptr<ICommandLogic>;