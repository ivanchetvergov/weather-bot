#include "KafkaConsumer.h" 
#include <iostream>        
#include <chrono>  // poll

using std::string;

KafkaConsumer::KafkaConsumer(const string& brokerList, const string& topic, const string& groupId)
    : config_({{"metadata.broker.list", brokerList}, // Адреса брокеров Kafka
                {"group.id", groupId},               // ID группы консьюмера
                {"auto.offset.reset", "earliest"},   // Начать с самого раннего сообщения, если нет сохраненного смещения
                {"enable.auto.commit", true},        // Включить автоматический коммит смещений
                {"auto.commit.interval.ms", 5000}}),  // Интервал автокоммита
      consumer_(std::make_shared<cppkafka::Consumer>(config_)), // Создаем объект Consumer
      running_(false) // Флаг состояния, изначально false (не запущен)
{
    consumer_->subscribe({topic});
    std::cout << "KafkaConsumer initialized and subscribed to topic: " << topic << std::endl;
}

KafkaConsumer::~KafkaConsumer() {
    stop();
}

// Метод для запуска Kafka Consumer
void KafkaConsumer::start(MessageHandler handler) {
    if (running_) {
        std::cerr << "KafkaConsumer is already running." << std::endl;
        return;
    }

    message_handler_ = handler; // Сохраняем переданный обработчик сообщений
    running_ = true;            // Устанавливаем флаг в true
    // Запускаем новый поток, который будет выполнять метод consume_loop
    consumer_thread_ = std::thread(&KafkaConsumer::consume_loop, this);
    std::cout << "KafkaConsumer started in background thread." << std::endl;
}

void KafkaConsumer::stop() {
    if (!running_) {
        return; 
    }

    running_ = false; 
    if (consumer_thread_.joinable()) {
        consumer_thread_.join();
    }
    std::cout << "KafkaConsumer stopped." << std::endl;
}

void KafkaConsumer::consume_loop() {
    while (running_) {
        cppkafka::Message msg = consumer_->poll(std::chrono::milliseconds(100));

        if (!msg || msg.get_error()) {
            continue; // Нет сообщения или временная ошибка, продолжаем опрос
        }

        // Если есть обработчик сообщений, вызываем его для полученного сообщения
        if (message_handler_) {
            message_handler_(msg);
        }
    }
}