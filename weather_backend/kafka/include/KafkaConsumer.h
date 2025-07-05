#pragma once

#include <cppkafka/cppkafka.h>
#include <string>
#include <thread>
#include <memory>
#include <functional> 

using std::string;

class KafkaConsumer {
public:
    using MessageHandler = std::function<void(const cppkafka::Message&)>;

    KafkaConsumer(const string& brokerList, const string& topic, const string& groupId);
    ~KafkaConsumer();

    void start(MessageHandler handler);
    void stop();

private:
    cppkafka::Configuration config_; // Конфигурация Kafka
    std::shared_ptr<cppkafka::Consumer> consumer_; // Объект Kafka Consumer
    std::thread consumer_thread_; // Поток для цикла потребления сообщений
    bool running_; // Флаг для управления циклом потока
    MessageHandler message_handler_; // Колбэк-функция для обработки сообщений

    // Основной цикл потребления сообщений Kafka
    void consume_loop();
};