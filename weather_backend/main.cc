#include <drogon/drogon.h>
#include <iostream>
#include <string>
#include <memory>   // Для std::unique_ptr
#include <functional> // Для std::bind
#include <fstream>

#include "KafkaConsumer.h" 
#include "KafkaMessageService.h" 

#include <locale>
#include <clocale>

const char* path = "/Users/ivan/prog/weather_bot/weather_backend/config.json";
const std::string KAFKA_BROKER_LIST = "localhost:9092";
const std::string KAFKA_COMMANDS_TOPIC = "bot_commands";

std::unique_ptr<KafkaConsumer> kafkaConsumerPtr;
std::unique_ptr<KafkaMessageService> kafkaMessageServicePtr;

int main() {

    std::ifstream file(path);
    if (!file.good()) {
        std::cerr << "ERROR: config.json not found at " << path << std::endl;
        return 1; 
    }
    file.close();
    
    std::setlocale(LC_ALL, ""); 

    drogon::app().addListener("0.0.0.0", 5555);
    drogon::app().loadConfigFile(path);

    kafkaMessageServicePtr = std::make_unique<KafkaMessageService>();

    kafkaConsumerPtr = std::make_unique<KafkaConsumer>(KAFKA_BROKER_LIST, KAFKA_COMMANDS_TOPIC, "drogon-telegram-bot-consumer-group");
    kafkaConsumerPtr->start(std::bind(&KafkaMessageService::processMessage, kafkaMessageServicePtr.get(), std::placeholders::_1));

    drogon::app().run();

    if (kafkaConsumerPtr) {
        kafkaConsumerPtr->stop();
    }

    std::cout << "Application gracefully stopped." << std::endl;
    return 0;
}