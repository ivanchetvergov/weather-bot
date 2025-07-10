#include "MyCityCommand.h"
#include <iostream>
#include <string>
#include <optional> 
#include "DataTransferObjects.h" 
#include "TelegramUpdateParser.h" // Убедись, что этот файл существует и в нем есть extractCityArgumentFromCommand

// Убедись, что ты используешь using namespace std; или std:: явно
using namespace std;

MyCityCommandLogic::MyCityCommandLogic(KafkaResponseSenderPtr response_sender, PgDbServicePtr dbService)
    : responseSender_(response_sender),
      dbService_(dbService)
{
    if (!responseSender_) {
        cerr << "ERROR: MyCityCommandLogic initialized with null response sender!" << endl;
    }
    if (!dbService_) {
        cerr << "ERROR: MyCityCommandLogic initialized with null DB service!" << endl;
    }
    cout << "MyCityCommandLogic initialized." << endl;
}

void MyCityCommandLogic::execute(const nlohmann::json& payload,
                                long long telegram_user_id,
                                const string& command_text, 
                                const string& username,
                                const string& first_name) {
    
    cout << "    MyCityCommandLogic: Handling /mycity command for Telegram User ID: " << telegram_user_id << endl;

    if (!this->dbService_) {
        cerr << "    ERROR: DB service is NULL in MyCityCommandLogic::execute. Cannot process command." << endl;
        if (responseSender_) {
             // Убрано MarkdownV2, если ты не используешь его
             responseSender_->sendTelegramMessage(telegram_user_id, "Произошла внутренняя ошибка. Пожалуйста, попробуйте позже.");
        }
        return;
    }

    if (telegram_user_id == 0) {
        cerr << "    ERROR: Cannot process /mycity for user with invalid Telegram User ID (0)." << endl;
        if (responseSender_) {
             responseSender_->sendTelegramMessage(telegram_user_id, "Ошибка: Не удалось определить ваш ID пользователя."); 
        }
        return;
    }

    std::optional<std::string> city_arg_opt = TelegramUpdateParser::extractCityArgumentFromCommand(command_text); 

    if (!city_arg_opt.has_value() || city_arg_opt.value().empty()) { 
        cerr << "    WARNING: MyCityCommandLogic received command without city argument. User ID: " << telegram_user_id << endl;
        if (responseSender_) {
            responseSender_->sendTelegramMessage(telegram_user_id, 
                                                 "Пожалуйста, укажите город по умолчанию. Пример: /mycity Москва"); 
        }
        return; 
    }

    string city_arg = city_arg_opt.value();

    UserData user_data; 
    user_data.telegram_user_id = telegram_user_id;
    user_data.username = username;
    user_data.first_name = first_name;
    user_data.default_city = city_arg; 

    this->dbService_->upsertUser(user_data);

    if (responseSender_) {
        cout << "    MyCityCommandLogic: Successfully initiated setting default city for user " << telegram_user_id 
                  << " to '" << city_arg << "'" << endl;
        responseSender_->sendTelegramMessage(telegram_user_id, 
                                             "Город по умолчанию установлен на " + city_arg + ".");
    }
}