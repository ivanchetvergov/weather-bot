#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class WeatherCacheController : public HttpController<WeatherCacheController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(WeatherCacheController::get, "/weather_cache/{1}", Get);
        ADD_METHOD_TO(WeatherCacheController::put, "/weather_cache", Post);
    METHOD_LIST_END

    void get(const HttpRequestPtr& req,
             std::function<void(const HttpResponsePtr&)>&& callback,
             std::string city);

    void put(const HttpRequestPtr& req,
             std::function<void(const HttpResponsePtr&)>&& callback);
};
