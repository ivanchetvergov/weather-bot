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
    cppkafka::Configuration config_; 
    std::shared_ptr<cppkafka::Consumer> consumer_; 
    std::thread consumer_thread_; 
    bool running_; 
    MessageHandler message_handler_; 

    void consume_loop();
};

