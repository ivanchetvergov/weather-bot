#include <drogon/drogon.h>
#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <fstream>
#include <locale>
#include <clocale>

#include "KafkaConsumer.h"
#include "KafkaMessageService.h"
#include "KafkaResponseSender.h"
#include "KafkaProducer.h"

#include "StartCommand.h"
// #include "services/command_logic/include/WeatherCommandLogic.h"
// #include "services/command_logic/include/TelegramMessageLogic.h"

const std::string KAFKA_BROKER_LIST = "localhost:9092";
const std::string KAFKA_COMMANDS_TOPIC = "bot_commands";
const std::string KAFKA_RESPONSES_TOPIC = "bot_responses";

const char* DROGON_CONFIG_PATH = "/Users/ivan/prog/weather_bot/weather_backend/config.json";

// global util kafka ptr's
std::unique_ptr<KafkaConsumer> kafkaConsumerPtr;
std::unique_ptr<KafkaMessageService> kafkaMessageServicePtr;
std::shared_ptr<KafkaProducer> kafkaProducerPtr;
std::shared_ptr<KafkaResponseSender> responseSenderPtr;

int main() {
    std::ifstream file(DROGON_CONFIG_PATH);
    if (!file.good()) {
        std::cerr << "ERROR: config.json not found at " << DROGON_CONFIG_PATH << std::endl;
        return 1;
    }
    file.close();

    std::setlocale(LC_ALL, "");

    drogon::app().addListener("0.0.0.0", 5555);
    drogon::app().loadConfigFile(DROGON_CONFIG_PATH);

    kafkaProducerPtr = std::make_shared<KafkaProducer>(KAFKA_BROKER_LIST);    

    kafkaMessageServicePtr = std::make_unique<KafkaMessageService>();
    kafkaMessageServicePtr->set_DBClientName("default");

    responseSenderPtr = std::make_shared<KafkaResponseSender>(kafkaProducerPtr, KAFKA_RESPONSES_TOPIC);
    kafkaMessageServicePtr->set_ResponseSender(responseSenderPtr);

    std::shared_ptr<StartCommandLogic> startLogic = std::make_shared<StartCommandLogic>(responseSenderPtr); // <-- ИЗМЕНЕНО
    kafkaMessageServicePtr->registerCommandLogic("/start", startLogic);

    kafkaConsumerPtr = std::make_unique<KafkaConsumer>(KAFKA_BROKER_LIST, KAFKA_COMMANDS_TOPIC, "drogon-telegram-bot-consumer-group");
    kafkaConsumerPtr->start(std::bind(&KafkaMessageService::processMessage, kafkaMessageServicePtr.get(), std::placeholders::_1));

    drogon::app().run();

    if (kafkaConsumerPtr) {
        kafkaConsumerPtr->stop();
    }

    std::cout << "Application gracefully stopped." << std::endl;
    return 0;
}