#include "StartCommand.h"
#include <iostream>
#include <string>

using namespace std;

StartCommandLogic::StartCommandLogic(KafkaResponseSenderPtr response_sender)
    : responseSender_(response_sender) {
    if (!responseSender_) {
        cerr << "ERROR: StartCommandLogic initialized with null response sender!" << endl;
    }
    cout << "StartCommandLogic initialized." << endl;
}

void StartCommandLogic::execute(drogon::orm::DbClientPtr db_client, // <-- ИЗМЕНЕНО: db_client здесь
                                const nlohmann::json& payload,
                                long long telegram_user_id,
                                const string& message_text,
                                const string& username,
                                const string& first_name) {
    cout << "    StartCommandLogic: Handling /start command for Telegram User ID: " << telegram_user_id << endl;

    if (!db_client) {
        cerr << "    ERROR: DB client is NULL in StartCommandLogic::execute. KafkaMessageService failed to get it." << endl;
        if (responseSender_) {
             responseSender_->sendTelegramMessage(telegram_user_id, "Произошла внутренняя ошибка базы данных. Попробуйте позже.");
        }
        return;
    }

    if (telegram_user_id == 0) {
        cerr << "    ERROR: Cannot register user with invalid Telegram User ID (0)." << endl;
        if (responseSender_) {
             responseSender_->sendTelegramMessage(telegram_user_id, "Ошибка: Не удалось определить ваш ID пользователя.");
        }
        return;
    }

    db_client->execSqlAsync(
        "INSERT INTO users (id, username, first_name) VALUES ($1, $2, $3) "
        "ON CONFLICT (id) DO UPDATE SET username = EXCLUDED.username, first_name = EXCLUDED.first_name, created_at = CURRENT_TIMESTAMP",

        [telegram_user_id, username, first_name, this](const drogon::orm::Result& result) {
            cout << "    User " << telegram_user_id
                      << " (" << (!username.empty() ? username : "N/A")
                      << ", " << first_name << ") registered/updated successfully in DB." << endl;

            if (responseSender_) {
                string welcomeMessage = "Привет, " + first_name + "! Я твой личный погодный бот. Используй /weather, чтобы узнать погоду.";
                responseSender_->sendTelegramMessage(telegram_user_id, welcomeMessage);
            }
        },

        [telegram_user_id, this](const drogon::orm::DrogonDbException& e) {
            cerr << "    ERROR: Failed to register/update user " << telegram_user_id
                      << " in DB: " << e.base().what() << endl;
            if (responseSender_) {
                responseSender_->sendTelegramMessage(telegram_user_id, "Извините, не удалось вас зарегистрировать из-за ошибки базы данных.");
            }
        },
        telegram_user_id,
        username.empty() ? nullptr : username.c_str(),
        first_name
    );
}