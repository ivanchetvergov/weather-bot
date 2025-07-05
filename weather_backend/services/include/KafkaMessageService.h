#pragma once

#include <cppkafka/cppkafka.h> // Для cppkafka::Message
#include <nlohmann/json.hpp>   // Для nlohmann::json
#include <string>              // Для std::string
#include <iostream>            // Для std::cout, std::cerr

// Здесь можно было бы включить другие сервисы, если KafkaMessageService
// будет вызывать их методы (например, WeatherService)
// #include "services/include/WeatherService.h"

class KafkaMessageService {
public:
    // Конструктор
    // Если этому сервису понадобятся зависимости (например, WeatherService),
    // их можно передать сюда через DI (Dependency Injection).
    KafkaMessageService();
    // KafkaMessageService(std::shared_ptr<WeatherService> weatherService);

    // Метод для обработки одного Kafka-сообщения
    void processMessage(const cppkafka::Message& msg);

private:
    // Если есть зависимости, хранить их здесь
    // std::shared_ptr<WeatherService> weatherService_;

    void handleWeatherCommand(const nlohmann::json& payload, const std::string& rawPayload);
    void handleTelegramMessage(const nlohmann::json& payload, const std::string& rawPayload);
};