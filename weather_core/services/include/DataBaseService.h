//DataBaseService.h
#pragma once

#include <drogon/orm/DbClient.h>
#include <drogon/orm/Exception.h>
#include <drogon/orm/Mapper.h>    
#include <string>
#include <memory>
#include <future> 
#include <functional> //

#include "DataTransferObjects.h"

#include "Users.h" 
#include "Messages.h"
#include "Subscriptions.h"
#include "Alerts.h"
#include "WeatherCache.h"

class PgDbService {
public:
    PgDbService(drogon::orm::DbClientPtr db_client);

    std::future<void> upsertUser(const UserData& user_data);
    std::future<void> insertMessage(const MessageData& message_data);
    std::future<void> insertSubscription(const SubscriptionData& sub_data);
    std::future<void> insertAlert(const AlertData& alert_data);
    std::future<void> upsertWeatherCache(const WeatherCacheData& cache_data);

    std::future<void> setUserDefaultCity(long long telegram_user_id,
                                         const std::string& city);

    void getUserDefaultCity(long long telegram_user_id,
                            std::function<void(std::optional<std::string>)> callback,
                            std::function<void(const std::exception&)> error_callback);

    void setUserDefaultCity(long long telegram_user_id,
                            const std::string& city,
                            std::function<void()> success_callback,   
                            std::function<void(const std::exception&)> error_callback); 


private:
    drogon::orm::DbClientPtr dbClient_;

    static std::future<void> handleDbClientNotAvailable(std::shared_ptr<std::promise<void>> prom);

    static void attachErrorLogger(std::future<void>&& fut, const std::string& operation_name);

};

using PgDbServicePtr = std::shared_ptr<PgDbService>;