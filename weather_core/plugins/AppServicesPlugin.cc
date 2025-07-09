/**
 *
 *  AppServices.cc
 *
 */

#include "AppServices.h"

using namespace drogon;

#include <iostream>

AppServices::AppServices(const Json::Value &config) {
    kafkaBrokerList_ = config.get("kafka_broker_list", "localhost:9092").asString();
    kafkaCommandsTopic_ = config.get("kafka_commands_topic", "bot_commands").asString();
    kafkaResponsesTopic_ = config.get("kafka_responses_topic", "bot_responses").asString();
    openWeatherApiKey_ = config.get("openweather_api_key", "").asString();
}

void AppServices::initAndStart(const Json::Value &config){
    std::cout << "AppServices::initAndStart() called. Attempting to get DbClient..." << std::endl;
    try {
        dbClient_ = drogon::app().getDbClient("default");
        if (!dbClient_) {
            LOG_FATAL << "Failed to get database client 'default' in AppServices::initAndStart()! Check config.json.";
            drogon::app().quit();
            return;
        }
        std::cout << "DbClient 'default' successfully retrieved in AppServices::initAndStart()." << std::endl;

        // --- ИНИЦИАЛИЗАЦИЯ ВСЕХ СЕРВИСОВ ---
        
        // 1. Инициализируем PgDbService
        dbServicePtr_ = std::make_shared<PgDbService>(dbClient_);
        
        // 2. Инициализируем KafkaProducer
        kafkaProducerPtr_ = std::make_shared<KafkaProducer>(kafkaBrokerList_);
        
        // 3. Инициализируем KafkaResponseSender
        responseSenderPtr_ = std::make_shared<KafkaResponseSender>(kafkaProducerPtr_, kafkaResponsesTopic_); 
        
        // 4. Инициализируем KafkaMessageService БЕЗ АРГУМЕНТОВ, затем устанавливаем зависимости
        kafkaMessageServicePtr_ = std::make_shared<KafkaMessageService>();
        kafkaMessageServicePtr_->set_ResponseSender(responseSenderPtr_);
        kafkaMessageServicePtr_->set_DbService(dbServicePtr_); // У тебя есть inline функция set_DbService, используем ее.
        
        // 5. Инициализируем логику команд с правильными аргументами
        startCommandLogic_ = std::make_shared<StartCommandLogic>(responseSenderPtr_, dbServicePtr_);
        weatherCommandLogic_ = std::make_shared<WeatherCommandLogic>(responseSenderPtr_, dbServicePtr_, openWeatherApiKey_); 

        // 6. Регистрируем обработчики команд в KafkaMessageService
        kafkaMessageServicePtr_->registerCommandLogic("start", startCommandLogic_);
        kafkaMessageServicePtr_->registerCommandLogic("weather", weatherCommandLogic_);

        // 7. Инициализируем KafkaConsumer (с groupId, который присутствует в твоем KafkaConsumer.h)
        kafkaConsumerPtr_ = std::make_shared<KafkaConsumer>(kafkaBrokerList_, kafkaCommandsTopic_, "drogon-telegram-bot-consumer-group");
        
    } catch (const drogon::orm::DrogonDbException& e) {
        LOG_FATAL << "Database error during AppServices initialization: " << e.base().what(); // Возвращено на .what()
        drogon::app().quit();
        return;
    } catch (const std::exception& e) {
        LOG_FATAL << "General error during AppServices initialization: " << e.what();
        drogon::app().quit();
        return;
    }

    startKafkaConsumer();
    std::cout << "Kafka Consumer started in AppServices::initAndStart()." << std::endl;
}


void AppServices::shutdown()
{
    std::cout << "AppServices::shutdown() called. Stopping Kafka Consumer..." << std::endl;
    stopKafkaConsumer();
    std::cout << "Kafka Consumer stopped." << std::endl;

}

void AppServices::startKafkaConsumer() {
    if (kafkaConsumerPtr_) {
        kafkaConsumerPtr_->start(std::bind(&KafkaMessageService::processMessage, kafkaMessageServicePtr_.get(), std::placeholders::_1));
        std::cout << "Kafka Consumer started." << std::endl;
    } else {
        std::cerr << "ERROR: Kafka Consumer not initialized." << std::endl;
    }
}

void AppServices::stopKafkaConsumer() {
    if (kafkaConsumerPtr_) {
        kafkaConsumerPtr_->stop();
        std::cout << "Kafka Consumer stopped." << std::endl;
    }
}