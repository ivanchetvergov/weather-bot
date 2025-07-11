#include "KafkaResponseSender.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <utility> 

using namespace std; // move, string ...

KafkaResponseSender::KafkaResponseSender(KafkaProducerPtr kafka_producer, const string& response_topic)
    : kafka_producer_(std::move(kafka_producer)),
      response_topic_(response_topic) {
    if (!kafka_producer_) {
        cerr << "ERROR: KafkaResponseSender initialized with null KafkaProducerPtr!" << endl;
    }
    cout << "KafkaResponseSender initialized for topic: " << response_topic_ << endl;
}


void KafkaResponseSender::sendTelegramMessage(long long user_id, const string& text) {
    if (!kafka_producer_) {
        cerr << "ERROR: KafkaProducer is not available in KafkaResponseSender. Cannot send Telegram message." << endl;
        return;
    }

    nlohmann::json payload;
    payload["event_type"] = "send_message";
    payload["telegram_user_id"] = user_id;
    payload["message"] = text;
    payload["source"] = "weather_backend";

    string message_json_str = payload.dump();
    string kafka_key = to_string(user_id);

    // Теперь вызываем метод produce на нашем kafka_producer_
    bool success = kafka_producer_->produce(response_topic_, kafka_key, message_json_str);
    if (success) {
        cout << "    Sent Telegram message to user " << user_id
                  << " to Kafka topic '" << response_topic_ << "' (via KafkaProducer)." << endl;
    } else {
        cerr << "    ERROR: Failed to send Telegram message to Kafka via KafkaProducer." << endl;
    }
}