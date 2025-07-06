#pragma once

#include "KafkaProducer.h" //
#include <string>
#include <memory> 

using namespace std;

class KafkaResponseSender { 
public:

    KafkaResponseSender(KafkaProducerPtr kafka_producer, const string& response_topic);

    ~KafkaResponseSender() = default;

    void sendTelegramMessage(long long user_id, const string& text);

private:
    KafkaProducerPtr kafka_producer_; 
    string response_topic_;
};

using KafkaResponseSenderPtr = shared_ptr<KafkaResponseSender>;