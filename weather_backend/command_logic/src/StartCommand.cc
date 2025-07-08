#include "StartCommand.h"
#include <iostream>
#include <string>

#include "DataTransferObjects.h"

using namespace std;

StartCommandLogic::StartCommandLogic(KafkaResponseSenderPtr response_sender, PgDbServicePtr dbService) // **Изменено: добавлен dbService**
    : responseSender_(response_sender),
      dbService_(dbService) 
{
    if (!responseSender_) {
        cerr << "ERROR: StartCommandLogic initialized with null response sender!" << endl;
    }
    if (!dbService_) {
        cerr << "ERROR: StartCommandLogic initialized with null DB service!" << endl;
    }
    cout << "StartCommandLogic initialized." << endl;
}

void StartCommandLogic::execute(const nlohmann::json& payload,
                                long long telegram_user_id,
                                const string& message_text,
                                const string& username,
                                const string& first_name) {
    cout << "    StartCommandLogic: Handling /start command for Telegram User ID: " << telegram_user_id << endl;

    if (!this->dbService_) {
        cerr << "    ERROR: DB service is NULL in StartCommandLogic::execute. Cannot process command." << endl;
        if (responseSender_) {
             responseSender_->sendTelegramMessage(telegram_user_id, "Произошла внутренняя ошибка. Попробуйте позже.");
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

    UserData user_data;
    user_data.telegram_user_id = telegram_user_id;
    user_data.username = username;
    user_data.first_name = first_name;

    this->dbService_->upsertUser(user_data);

    if (responseSender_) {
        string welcomeMessage = "Привет, " + first_name + "! Я твой личный погодный бот. Используй /weather, чтобы узнать погоду.";
        responseSender_->sendTelegramMessage(telegram_user_id, welcomeMessage);
    }
}