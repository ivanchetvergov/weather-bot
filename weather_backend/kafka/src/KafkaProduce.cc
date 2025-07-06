#include "KafkaProducer.h"
#include <iostream>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::make_unique;

KafkaProducer::KafkaProducer(const string& broker_list)
    : config_({{"metadata.broker.list", broker_list}}),
      producer_(make_unique<cppkafka::Producer>(config_)) { 
    cout << "KafkaProducer initialized with brokers: " << broker_list << endl;
}

KafkaProducer::~KafkaProducer() {
    if (producer_) {
        producer_->flush(std::chrono::seconds(10));
        cout << "KafkaProducer: All pending messages flushed." << endl;
    }
}

bool KafkaProducer::produce(const string& topic, const string& key, const string& payload) {
    if (!producer_) {
        cerr << "ERROR: KafkaProducer is not initialized. Cannot produce message to topic '" << topic << "'." << endl;
        return false;
    }
    try {
        producer_->produce(cppkafka::MessageBuilder(topic).key(key).payload(payload));
        return true;
    } catch (const cppkafka::Exception& e) { 
        cerr << "ERROR: KafkaProducer failed to produce message to topic '" << topic
                  << "' with key '" << key << "': " << e.what() << endl;
        return false;
    }
}

bool KafkaProducer::produce(const string& topic, const string& payload) {
    if (!producer_) {
        cerr << "ERROR: KafkaProducer is not initialized. Cannot produce message to topic '" << topic << "'." << endl;
        return false;
    }
    try {
        producer_->produce(cppkafka::MessageBuilder(topic).payload(payload));
        return true;
    } catch (const cppkafka::Exception& e) { 
        cerr << "ERROR: KafkaProducer failed to produce message to topic '" << topic
                  << "': " << e.what() << endl;
        return false;
    }
}