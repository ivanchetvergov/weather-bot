#include "AppServices.h"

using namespace drogon;

#include <iostream>

AppServices::AppServices(const Json::Value &config)
{
    kafkaBrokerList_ = config.get("kafka_broker_list", "localhost:9092").asString();
    kafkaCommandsTopic_ = config.get("kafka_commands_topic", "bot_commands").asString();
    kafkaResponsesTopic_ = config.get("kafka_responses_topic", "bot_responses").asString();
    openWeatherApiKey_ = config.get("openweather_api_key", "").asString();

    kafkaConsumerPtr_ = nullptr;
    kafkaMessageServicePtr_ = nullptr;
    kafkaProducerPtr_ = nullptr;
    responseSenderPtr_ = nullptr;
    dbClient_ = nullptr;
    startCommandLogic_ = nullptr;
    weatherCommandLogic_ = nullptr;
}

void AppServices::initializeNonDbServices()
{
    std::cout << "AppServices::initializeNonDbServices() called. Initializing non-DB services..." << std::endl;
    try {
        kafkaProducerPtr_ = std::make_shared<KafkaProducer>(kafkaBrokerList_);
        responseSenderPtr_ = std::make_shared<KafkaResponseSender>(kafkaProducerPtr_, kafkaResponsesTopic_); 
        
        kafkaMessageServicePtr_ = std::make_shared<KafkaMessageService>();
        kafkaMessageServicePtr_->set_ResponseSender(responseSenderPtr_);

    } catch (const std::exception& e) {
        LOG_FATAL << "General error during AppServices non-DB initialization: " << e.what();
        drogon::app().quit();
        return;
    }
    std::cout << "Non-DB AppServices initialized successfully." << std::endl;
}

void AppServices::initializeDbAndDependentServices()
{
    std::cout << "AppServices::initializeDbAndDependentServices() called. Attempting to get DbClient and dependent services..." << std::endl;
    try {
        dbClient_ = drogon::app().getDbClient("default");
        if (!dbClient_) {
            LOG_FATAL << "Failed to get database client 'default' in AppServices::initializeDbAndDependentServices()! Check config.json and DB connection.";
            drogon::app().quit();
            return;
        }
        std::cout << "DbClient 'default' successfully retrieved." << std::endl;

        dbServicePtr_ = std::make_shared<PgDbService>(dbClient_);
        std::cout << "PgDbService successfully initialized." << std::endl;

        if (kafkaMessageServicePtr_) {
            kafkaMessageServicePtr_->set_DbService(dbServicePtr_); 
            std::cout << "dbServicePtr_ set in KafkaMessageService." << std::endl;
        } else {
             LOG_FATAL << "KafkaMessageSmyCityCommandLogic_ervice not initialized before trying to set DbService.";
             drogon::app().quit();
             return;
        }
        
        startCommandLogic_ = std::make_shared<StartCommandLogic>(responseSenderPtr_, dbServicePtr_);
        weatherCommandLogic_ = std::make_shared<WeatherCommandLogic>(responseSenderPtr_, dbServicePtr_, openWeatherApiKey_);
        forecastCommandLogic_ = std::make_shared<ForecastCommandLogic>(responseSenderPtr_, dbServicePtr_, openWeatherApiKey_);  
        myCityCommandLogic_ = std::make_shared<MyCityCommandLogic>(responseSenderPtr_, dbServicePtr_);  

        kafkaMessageServicePtr_->registerCommandLogic("/start", startCommandLogic_);
        kafkaMessageServicePtr_->registerCommandLogic("/weather", weatherCommandLogic_);
        kafkaMessageServicePtr_->registerCommandLogic("/forecast", forecastCommandLogic_);
        kafkaMessageServicePtr_->registerCommandLogic("/mycity", myCityCommandLogic_);

        kafkaConsumerPtr_ = std::make_shared<KafkaConsumer>(kafkaBrokerList_, kafkaCommandsTopic_, "drogon-telegram-bot-consumer-group");
        
    } catch (const drogon::orm::DrogonDbException& e) {
        LOG_FATAL << "Database error during AppServices DB initialization: " << e.base().what();
        drogon::app().quit();
        return;
    } catch (const std::exception& e) {
        LOG_FATAL << "General error during AppServices DB initialization: " << e.what();
        drogon::app().quit();
        return;
    }

    startKafkaConsumer();
    std::cout << "Kafka Consumer started." << std::endl;
    std::cout << "All AppServices (including DB and dependent services) initialized successfully." << std::endl;
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