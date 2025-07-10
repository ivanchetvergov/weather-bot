//DataBaseService.h
#pragma once

#include <drogon/orm/DbClient.h>
#include <drogon/orm/Exception.h> 
#include <drogon/orm/Mapper.h>    
#include <string>
#include <memory>
#include <future> 

#include "DataTransferObjects.h"

#include "Users.h" 
#include "Messages.h"
#include "Subscriptions.h"
#include "Alerts.h"
#include "WeatherCache.h"

using std::future;
using std::promise;
using std::string;
using std::shared_ptr;

class PgDbService {
public:
    PgDbService(drogon::orm::DbClientPtr db_client);

    future<void> upsertUser(const UserData& user_data);
    future<void> insertMessage(const MessageData& message_data);
    future<void> insertSubscription(const SubscriptionData& sub_data);
    future<void> insertAlert(const AlertData& alert_data);
    future<void> upsertWeatherCache(const WeatherCacheData& cache_data);

    future<std::optional<string>> getUserDefaultCity(long long telegram_user_id); 

private:
    drogon::orm::DbClientPtr dbClient_;
    static future<void> handleDbClientNotAvailable(shared_ptr<promise<void>> prom);
    static void attachErrorLogger(future<void>&& fut, const string& operation_name);
    void updateDefaultCityIfNeeded(drogon_model::weather_bot::Users& user, const std::optional<std::string>& new_default_city, bool& needs_update);
};

using PgDbServicePtr = shared_ptr<PgDbService>;