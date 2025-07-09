#pragma once
#include <string>
#include <TelegramService.h>

class NotificationService {
public:
    static void checkAndNotify();

    static void sendWeatherAlert(int64_t chatId, const std::string& city, const std::string& condition) {
        std::string message = "Погода в " + city + ": " + condition;
        TelegramService::sendMessage(chatId, message);
    }

    static void sendDailySummary(int64_t chatId, const std::string& city, const std::string& summary) {
        std::string message = "Ежедневная сводка по " + city + ":\n" + summary;
        TelegramService::sendMessage(chatId, message);
    }

    static void sendSubscriptionConfirmation(int64_t chatId, const std::string& city) {
        std::string message = "Подписка на уведомления по городу " + city + " успешно оформлена.";
        TelegramService::sendMessage(chatId, message);
    }
};
