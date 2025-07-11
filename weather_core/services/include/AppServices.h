// AppService.h

#pragma once

#include <drogon/orm/DbClient.h>
#include <memory>
#include <string>
#include <json/json.h>

#include "KafkaConsumer.h"
#include "KafkaMessageService.h"
#include "KafkaResponseSender.h"
#include "KafkaProducer.h"
#include "DataBaseService.h"

#include "StartCommand.h"
#include "WeatherCommand.h"
#include "ForecastCommand.h"
#include "MyCityCommand.h"

class AppServices
{
  public:
    AppServices(const Json::Value &config); 
    
    void shutdown(); 
    void initializeNonDbServices();
    void initializeDbAndDependentServices(); 

    void startKafkaConsumer();
    void stopKafkaConsumer(); 

    std::shared_ptr<PgDbService> getDbService() const { return dbServicePtr_; }
    std::shared_ptr<KafkaMessageService> getKafkaMessageService() const { return kafkaMessageServicePtr_; }

private:
    std::string kafkaBrokerList_;
    std::string kafkaCommandsTopic_;
    std::string kafkaResponsesTopic_;
    std::string openWeatherApiKey_; 

    std::shared_ptr<PgDbService> dbServicePtr_;
    std::shared_ptr<KafkaConsumer> kafkaConsumerPtr_;
    std::shared_ptr<KafkaMessageService> kafkaMessageServicePtr_;
    std::shared_ptr<KafkaProducer> kafkaProducerPtr_;
    std::shared_ptr<KafkaResponseSender> responseSenderPtr_;

    drogon::orm::DbClientPtr dbClient_;
    std::shared_ptr<ForecastCommandLogic> forecastCommandLogic_;
    std::shared_ptr<StartCommandLogic> startCommandLogic_;
    std::shared_ptr<WeatherCommandLogic> weatherCommandLogic_; 
    std::shared_ptr<MyCityCommandLogic> myCityCommandLogic_;
};


