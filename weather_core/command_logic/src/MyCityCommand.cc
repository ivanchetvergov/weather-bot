// MyCityCommand.cc

#include "MyCityCommand.h" 
#include <iostream>
#include <string>
#include <optional> 
#include "DataTransferObjects.h" 
#include "TelegramUpdateParser.h" 

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
                                         const std::string& message_text,
                                         const std::string& username,
                                         const std::string& first_name
) {
    
    std::string city_to_set = TelegramUpdateParser::extractCityArgumentFromCommand(message_text).value_or("");

    if (city_to_set.empty()) {
        responseSender_->sendTelegramMessage(telegram_user_id, "Пожалуйста, укажите город для установки по умолчанию. Пример: установи город по умолчанию Москва");
        return;
    }

    if (!dbService_) {
        std::cerr << "ERROR: DB service is NULL in MyCityCommandLogic::execute." << std::endl; // Исправлено имя класса
        responseSender_->sendTelegramMessage(telegram_user_id, "Произошла внутренняя ошибка. Попробуйте позже."); // Исправлен вызов
        return;
    }

    // Вызываем setUserDefaultCity с колбэками успеха и ошибки
    dbService_->setUserDefaultCity(telegram_user_id, city_to_set,
        // Колбэк успеха
        [this, telegram_user_id, city_to_set]() {
            responseSender_->sendTelegramMessage(telegram_user_id, "Город по умолчанию установлен на " + city_to_set + ".");
        },
        // Колбэк ошибки
        [this, telegram_user_id](const std::exception& e) {
            std::cerr << "ERROR: Failed to set default city for user " << telegram_user_id << ": " << e.what() << std::endl;
            responseSender_->sendTelegramMessage(telegram_user_id, "Произошла ошибка при установке города по умолчанию. Попробуйте позже."); // Исправлен вызов
        }
    );
}