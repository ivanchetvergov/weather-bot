#pragma once

#include <cppkafka/cppkafka.h>
#include <string>
#include <memory> // Для std::unique_ptr

class KafkaProducer {
public:

    KafkaProducer(const std::string& broker_list);
    ~KafkaProducer();

    bool produce(const std::string& topic, const std::string& key, const std::string& payload);

    bool produce(const std::string& topic, const std::string& payload);

private:
    cppkafka::Configuration config_;
    std::unique_ptr<cppkafka::Producer> producer_;
};

using KafkaProducerPtr = std::shared_ptr<KafkaProducer>;