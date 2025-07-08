// weather_backend/main.cc
#include <drogon/drogon.h>
#include <iostream>
#include <string>
#include <fstream>
#include <locale>
#include <memory>

#include "AppServices.h"
static AppServices dummyAppServices(Json::Value());

const char* DROGON_CONFIG_PATH = "/Users/ivan/prog/weather_bot/weather_backend/config.json";

int main() {
    std::ifstream file(DROGON_CONFIG_PATH);
    if (!file.good()) {
        std::cerr << "ERROR: config.json not found at " << DROGON_CONFIG_PATH << std::endl;
        return 1;
    }
    file.close();

    std::setlocale(LC_ALL, ""); // Настройка локали

    setDbSource

    drogon::app().addListener("0.0.0.0", 5555); // HTTP listener
    drogon::app().loadConfigFile(DROGON_CONFIG_PATH); // Загрузка config.json

    drogon::app().run();

    std::cout << "Application gracefully stopped." << std::endl;
    return 0;
}