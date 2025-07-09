// main.cc
#include <drogon/drogon.h>
#include <iostream>
#include <string>
#include <fstream>
#include <locale>
#include <memory>
#include <json/json.h>
#include <csignal> 

#include "AppServices.h" 

const char* DROGON_CONFIG_PATH = "/Users/ivan/prog/weather_bot/weather_core/config.json";

std::shared_ptr<AppServices> globalAppServices = nullptr;

void handleSignal(int signum) {
    std::cout << "Caught signal " << signum << ". Shutting down AppServices..." << std::endl;
    if (globalAppServices) {
        globalAppServices->shutdown();
    }
    drogon::app().quit(); 
}

int main() {
    std::ifstream file(DROGON_CONFIG_PATH);
    if (!file.good()) {
        std::cerr << "ERROR: config.json not found at " << DROGON_CONFIG_PATH << std::endl;
        return 1;
    }
    file.close(); 

    std::setlocale(LC_ALL, "");

    drogon::app().loadConfigFile(DROGON_CONFIG_PATH);

    Json::CharReaderBuilder builder;
    std::string errs;
    std::ifstream configFileStream(DROGON_CONFIG_PATH); 
    Json::Value root;
    if (!Json::parseFromStream(builder, configFileStream, &root, &errs)) {
        std::cerr << "ERROR: Failed to parse config.json: " << errs << std::endl;
        return 1;
    }

    Json::Value appServicesConfig;
    if (root.isMember("AppServices")) {
        appServicesConfig = root["AppServices"];
    } else {
        std::cerr << "ERROR: 'AppServices' configuration section not found in config.json! "
                  << "It should be a top-level key outside of 'plugins' array." << std::endl;
        return 1;
    }

    globalAppServices = std::make_shared<AppServices>(appServicesConfig);
    globalAppServices->initializeNonDbServices(); 

    drogon::app().registerBeginningAdvice([&]() {
        if (globalAppServices) {
            globalAppServices->initializeDbAndDependentServices();
        } else {
            LOG_FATAL << "globalAppServices is null in beginning advice callback. Critical error.";
            drogon::app().quit();
        }
    });
    signal(SIGINT, handleSignal);  
    signal(SIGTERM, handleSignal); 

    drogon::app().run();

    std::cout << "Application gracefully stopped." << std::endl;
    return 0;
}