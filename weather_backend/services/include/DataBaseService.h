#pragma once

#include <drogon/orm/DbClient.h>
#include <string>
#include <memory>
#include <future> 


#include "DataTransferObject.h"

using namespace std::future;

class PgDbService {
public:
    PgDbService(drogon::orm::DbClientPtr db_client);

    future<void> upsertUser(const UserData& user_data);
    future<void> insertMessage(const MessageData& message_data);
    future<void> insertSubscription(const SubscriptionData& sub_data);
    future<void> insertAlert(const AlertData& alert_data);
    future<void> upsertWeatherCache(const WeatherCacheData& cache_data);

private:
    drogon::orm::DbClientPtr dbClient_;
};

using PgDbServicePtr = std::shared_ptr<PgDbService>;