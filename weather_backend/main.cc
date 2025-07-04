#include <drogon/drogon.h>
#include <cppkafka/cppkafka.h>
#include <thread>
#include <memory>

void startKafkaConsumer() {
    using namespace cppkafka;

    Configuration config = {
        {"metadata.broker.list", "localhost:9092"},
        {"group.id", "drogon-consumer"},
        {"auto.offset.reset", "earliest"}
    };

    auto consumer = std::make_shared<Consumer>(config);
    consumer->subscribe({"telegram-messages"});

    std::thread([consumer]() {
        while (true) {
            auto msg = consumer->poll();
            if (msg) {
                std::cout << msg.get_payload() << std::endl;
            }
        }
    }).detach();
}

int main() {
    //Set HTTP listener address and port
    drogon::app().addListener("0.0.0.0", 5555);
    //Load config file
    drogon::app().loadConfigFile("../config.json");

    startKafkaConsumer();

    //drogon::app().loadConfigFile("../config.yaml");
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
