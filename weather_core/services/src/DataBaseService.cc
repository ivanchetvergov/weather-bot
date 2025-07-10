// DataBaseService.cc
#include "DataBaseService.h"

#include <drogon/drogon.h>
#include <iostream>
#include <utility> // Для std::move

using namespace drogon::orm;
using namespace drogon_model::weather_bot;

PgDbService::PgDbService(drogon::orm::DbClientPtr db_client)
    : dbClient_(std::move(db_client)) {
    if (!dbClient_) {
        std::cerr << "WARNING: PgDbService initialized with a null DbClientPtr." << std::endl;
    }
}

future<void> PgDbService::handleDbClientNotAvailable(std::shared_ptr<std::promise<void>> prom) {
    std::cerr << "ERROR: Database client is not available." << std::endl;
    prom->set_exception(std::make_exception_ptr(drogon::orm::BrokenConnection("Database client is not available.")));
    return prom->get_future(); 
}

future<void> PgDbService::upsertUser(const UserData& user_data) {
    auto prom = std::make_shared<std::promise<void>>();
    future<void> fut = prom->get_future(); 

    if (!dbClient_) {
        return handleDbClientNotAvailable(prom);
    }

    auto usersMapper = Mapper<Users>(dbClient_);
    usersMapper.findBy(Criteria(Users::Cols::_id, CompareOperator::EQ, user_data.telegram_user_id),
        [prom, user_data, usersMapper](const std::vector<Users>& users) mutable {
            if (!users.empty()) {
                auto user = users[0];
                bool needs_update = false;
                if (user.getValueOfUsername() != user_data.username) {
                    user.setUsername(user_data.username);
                    needs_update = true;
                }
                if (user.getValueOfFirstName() != user_data.first_name) {
                    user.setFirstName(user_data.first_name);
                    needs_update = true;
                }

                if (needs_update) {
                    // Обновляем существующего пользователя
                    usersMapper.update(user,
                        [prom](size_t updated_rows) {
                            prom->set_value(); // Успех
                        },
                        [prom](const DrogonDbException& e_update) {
                            // Логирование ошибки обновления
                            std::cerr << "ERROR in DB update (upsertUser): " << e_update.base().what() << std::endl;
                            if (auto sqlError = dynamic_cast<const SqlError*>(&e_update.base())) {
                                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
                            }
                            prom->set_exception(std::make_exception_ptr(e_update)); // Установка исключения
                        });
                } else {
                    prom->set_value();
                }
            } else {
                Users newUser;
                newUser.setId(user_data.telegram_user_id);
                newUser.setUsername(user_data.username);
                newUser.setFirstName(user_data.first_name);

                usersMapper.insert(newUser,
                    [prom](Users insertedUser) {
                        prom->set_value(); // Успех
                    },
                    [prom](const DrogonDbException& e_insert) {
                        // Логирование ошибки вставки
                        std::cerr << "ERROR in DB insert (upsertUser): " << e_insert.base().what() << std::endl;
                        if (auto sqlError = dynamic_cast<const SqlError*>(&e_insert.base())) {
                            std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
                        }
                        prom->set_exception(std::make_exception_ptr(e_insert)); // Установка исключения
                    });
            }
        },
        [prom](const DrogonDbException& e_find) {
            std::cerr << "ERROR in DB find (upsertUser): " << e_find.base().what() << std::endl;
            if (auto sqlError = dynamic_cast<const SqlError*>(&e_find.base())) {
                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
            }
            prom->set_exception(std::make_exception_ptr(e_find)); // Установка исключения
        });

    return fut; 
}

future<void> PgDbService::insertMessage(const MessageData& message_data) {
    auto prom = std::make_shared<std::promise<void>>();
    std::future<void> fut = prom->get_future();

    if (!dbClient_) {
        return handleDbClientNotAvailable(prom);
    }

    auto messagesMapper = Mapper<Messages>(dbClient_);
    Messages newMessage;

    newMessage.setUserId(message_data.user_id);
    newMessage.setCommandText(message_data.command_text); // <--- Используем новое поле
    newMessage.setText(message_data.text);

    messagesMapper.insert(newMessage,
        [prom](Messages insertedMessage) {
            prom->set_value(); 
        },
        [prom](const DrogonDbException& e_insert) {
            std::cerr << "ERROR in DB insert (insertMessage): " << e_insert.base().what() << std::endl;
            if (auto sqlError = dynamic_cast<const SqlError*>(&e_insert.base())) {
                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
            }
            prom->set_exception(std::make_exception_ptr(e_insert));
        });

    return fut; 
}

// Subscriptions
future<void> PgDbService::insertSubscription(const SubscriptionData& sub_data) {
    auto prom = std::make_shared<std::promise<void>>();
    future<void> fut = prom->get_future(); 

    if (!dbClient_) {
        return handleDbClientNotAvailable(prom);
    }

    auto subMapper = Mapper<Subscriptions>(dbClient_);
    Subscriptions newSub;
    newSub.setUserId(sub_data.user_id);
    newSub.setCity(sub_data.city);

    if (sub_data.temp_above.has_value()) {
        newSub.setTempAbove(sub_data.temp_above.value());
    } else {
        newSub.setTempAboveToNull();
    }
    if (sub_data.rain.has_value()) {
        newSub.setRain(sub_data.rain.value());
    } else {
        newSub.setRainToNull();
    }
    if (sub_data.wind_speed_gt.has_value()) {
        newSub.setWindSpeedGt(sub_data.wind_speed_gt.value());
    } else {
        newSub.setWindSpeedGtToNull();
    }
    if (sub_data.notify_time.has_value()) {
        newSub.setNotifyTime(sub_data.notify_time.value());
    } else {
        newSub.setNotifyTimeToNull();
    }

    subMapper.insert(newSub,
        [prom](Subscriptions insertedSub) {
            prom->set_value(); // Успех
        },
        [prom, user_id = sub_data.user_id, city = sub_data.city](const DrogonDbException& e_insert) {
            if (auto uniqueViolation = dynamic_cast<const UniqueViolation*>(&e_insert.base())) {
                 std::cerr << "Subscription for user " << user_id << " in city " << city << " already exists (UniqueViolation)." << std::endl;
            } else {
                std::cerr << "ERROR in DB insert (insertSubscription): " << e_insert.base().what() << std::endl;
            }
            if (auto sqlError = dynamic_cast<const SqlError*>(&e_insert.base())) {
                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
            }
            prom->set_exception(std::make_exception_ptr(e_insert)); // Установка исключения
        });

    return fut;
}

// Alerts
future<void> PgDbService::insertAlert(const AlertData& alert_data) {
    auto prom = std::make_shared<std::promise<void>>();
    future<void> fut = prom->get_future(); 

    if (!dbClient_) {
        return handleDbClientNotAvailable(prom);
    }

    auto alertsMapper = Mapper<Alerts>(dbClient_);
    Alerts newAlert;
    newAlert.setUserId(alert_data.user_id);
    newAlert.setCity(alert_data.city);
    newAlert.setAlertCondition(alert_data.condition);

    alertsMapper.insert(newAlert,
        [prom](Alerts insertedAlert) {
            prom->set_value(); 
        },
        [prom](const DrogonDbException& e_insert) {
            std::cerr << "ERROR in DB insert (insertAlert): " << e_insert.base().what() << std::endl;
            if (auto sqlError = dynamic_cast<const SqlError*>(&e_insert.base())) {
                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
            }
            prom->set_exception(std::make_exception_ptr(e_insert));
        });

    return fut; 
}


future<void> PgDbService::upsertWeatherCache(const WeatherCacheData& cache_data) {
    auto prom = std::make_shared<std::promise<void>>();
    future<void> fut = prom->get_future();

    if (!dbClient_) {
        return handleDbClientNotAvailable(prom);
    }

    auto cacheMapper = Mapper<WeatherCache>(dbClient_);
    cacheMapper.findBy(Criteria(WeatherCache::Cols::_city, CompareOperator::EQ, cache_data.city),
        [prom, cache_data, cacheMapper](const std::vector<WeatherCache>& caches) mutable {
            if (!caches.empty()) {
                auto cache = caches[0];
                cache.setTimestamp(cache_data.timestamp);
                cache.setJsonData(cache_data.json_data);

                cacheMapper.update(cache,
                    [prom](size_t updated_rows) {
                        prom->set_value();
                    },
                    [prom](const DrogonDbException& e_update) {
                        std::cerr << "ERROR in DB update (upsertWeatherCache): " << e_update.base().what() << std::endl;
                        if (auto sqlError = dynamic_cast<const SqlError*>(&e_update.base())) {
                            std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
                        }
                        prom->set_exception(std::make_exception_ptr(e_update)); 
                    });
            } else {
                WeatherCache newCache;
                newCache.setCity(cache_data.city);
                newCache.setTimestamp(cache_data.timestamp);
                newCache.setJsonData(cache_data.json_data);

                cacheMapper.insert(newCache,
                    [prom](WeatherCache insertedCache) {
                        prom->set_value(); 
                    },
                    [prom](const DrogonDbException& e_insert) {
                        std::cerr << "ERROR in DB insert (upsertWeatherCache): " << e_insert.base().what() << std::endl;
                        if (auto sqlError = dynamic_cast<const SqlError*>(&e_insert.base())) {
                            std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
                        }
                        prom->set_exception(std::make_exception_ptr(e_insert));
                    });
            }
        },
        [prom](const DrogonDbException& e_find) {
            std::cerr << "ERROR in DB find (upsertWeatherCache): " << e_find.base().what() << std::endl;
            if (auto sqlError = dynamic_cast<const SqlError*>(&e_find.base())) {
                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
            }
            prom->set_exception(std::make_exception_ptr(e_find));
        });

    return fut; 
}