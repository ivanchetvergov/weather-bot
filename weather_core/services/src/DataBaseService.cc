// DataBaseService.cc
#include "DataBaseService.h"

#include <drogon/drogon.h> // Оставим, если он нужен для других частей Drogon
#include <iostream>
#include <utility> // Для std::move
#include <future> // Убедимся, что std::future и std::promise доступны здесь

using namespace drogon::orm; // Это поможет с DrogonDbException, Mapper, Criteria, CompareOperator, SqlError, UniqueViolation
using namespace drogon_model::weather_bot;

// Добавим using-директивы для std::, чтобы не писать std:: везде в этом файле
using std::future;
using std::promise;
using std::shared_ptr;
using std::string;
using std::optional; // Если используешь optional без std::

PgDbService::PgDbService(drogon::orm::DbClientPtr db_client)
    : dbClient_(std::move(db_client)) {
    if (!dbClient_) {
        std::cerr << "WARNING: PgDbService initialized with a null DbClientPtr." << std::endl;
    }
}

std::future<void> PgDbService::handleDbClientNotAvailable(std::shared_ptr<std::promise<void>> prom) {
    std::cerr << "ERROR: Database client is not available." << std::endl;
    prom->set_exception(std::make_exception_ptr(std::runtime_error("Database client is not available.")));
    return prom->get_future(); 
}

void PgDbService::getUserDefaultCity(long long telegram_user_id,
                                       std::function<void(std::optional<std::string>)> callback,
                                       std::function<void(const std::exception&)> error_callback) { // Сигнатура теперь правильная
    if (!dbClient_) {
        if (error_callback) {
            error_callback(std::runtime_error("Database client not available")); 
        }
        return;
    }

    auto usersMapper = Mapper<Users>(dbClient_);
    usersMapper.findBy(Criteria(Users::Cols::_id, CompareOperator::EQ, telegram_user_id),
        [callback, error_callback](const std::vector<Users>& users) {
            if (!users.empty()) {
                auto user = users[0];
                if (callback) {
                    callback(user.getValueOfDefaultCity());
                }
            } else {
                if (callback) {
                    callback(std::nullopt); 
                }
            }
        },
        [error_callback](const DrogonDbException& e) { // Эта лямбда все еще принимает DrogonDbException от Drogon
            std::cerr << "ERROR in DB find (getUserDefaultCity): " << e.base().what() << std::endl;
            if (auto sqlError = dynamic_cast<const SqlError*>(&e.base())) {
                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
            }
            if (error_callback) {
                error_callback(std::runtime_error(e.base().what()));
            }
        });
}

void PgDbService::setUserDefaultCity(long long telegram_user_id,
                                     const std::string& city,
                                     std::function<void()> success_callback,
                                     std::function<void(const std::exception&)> error_callback) {
    if (!dbClient_) {
        if (error_callback) {
            error_callback(std::runtime_error("Database client not available"));
        }
        return;
    }

    auto usersMapper = Mapper<Users>(dbClient_);
    usersMapper.findBy(Criteria(Users::Cols::_id, CompareOperator::EQ, telegram_user_id),
        [success_callback, error_callback, usersMapper, city, telegram_user_id](const std::vector<Users>& users) mutable {
            if (!users.empty()) {
                auto user = users[0];
                if (!user.getDefaultCity() || *user.getDefaultCity() != city) {
                    user.setDefaultCity(city);
                    usersMapper.update(user,
                        [success_callback](size_t updated_rows) {
                            if (success_callback) {
                                success_callback();
                            }
                        },
                        [error_callback](const DrogonDbException& e_update) {
                            std::cerr << "ERROR in DB update (setUserDefaultCity): " << e_update.base().what() << std::endl;
                            if (auto sqlError = dynamic_cast<const SqlError*>(&e_update.base())) {
                                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
                            }
                            if (error_callback) {
                                error_callback(std::runtime_error(e_update.base().what())); // Оборачиваем в runtime_error
                            }
                        });
                } else {
                    if (success_callback) {
                        success_callback();
                    }
                }
            } else {
                // Пользователь не найден. Вызываем ошибку
                std::cerr << "ERROR: User " << telegram_user_id << " not found for setting default city." << std::endl;
                if (error_callback) {
                    error_callback(std::runtime_error("User not found for setting default city."));
                }
            }
        },
        [error_callback](const DrogonDbException& e_find) {
            std::cerr << "ERROR in DB find (setUserDefaultCity): " << e_find.base().what() << std::endl;
            if (auto sqlError = dynamic_cast<const SqlError*>(&e_find.base())) {
                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
            }
            if (error_callback) {
                error_callback(std::runtime_error(e_find.base().what())); // Оборачиваем в runtime_error
            }
        });
}


// Теперь явно указываем std::future
std::future<void> PgDbService::upsertUser(const UserData& user_data) {
    auto prom = std::make_shared<std::promise<void>>();
    std::future<void> fut = prom->get_future(); 

    if (!dbClient_) {
        return handleDbClientNotAvailable(prom);
    }

    auto usersMapper = Mapper<Users>(dbClient_);
    usersMapper.findBy(Criteria(Users::Cols::_id, CompareOperator::EQ, user_data.telegram_user_id),
        [prom, user_data, usersMapper, this](const std::vector<Users>& users) mutable {
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
                    usersMapper.update(user,
                        [prom](size_t updated_rows) {
                            prom->set_value(); 
                        },
                        [prom](const DrogonDbException& e_update) {
                            std::cerr << "ERROR in DB update (upsertUser): " << e_update.base().what() << std::endl;
                            if (auto sqlError = dynamic_cast<const SqlError*>(&e_update.base())) {
                                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
                            }
                            prom->set_exception(std::make_exception_ptr(e_update)); 
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
                        prom->set_value(); 
                    },
                    [prom](const DrogonDbException& e_insert) {
                        std::cerr << "ERROR in DB insert (upsertUser): " << e_insert.base().what() << std::endl;
                        if (auto sqlError = dynamic_cast<const SqlError*>(&e_insert.base())) {
                            std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
                        }
                        prom->set_exception(std::make_exception_ptr(e_insert)); 
                    });
            }
        },
        [prom](const DrogonDbException& e_find) {
            std::cerr << "ERROR in DB find (upsertUser): " << e_find.base().what() << std::endl;
            if (auto sqlError = dynamic_cast<const SqlError*>(&e_find.base())) {
                std::cerr << "SQLSTATE: " << sqlError->sqlState() << ", Query: " << sqlError->query() << std::endl;
            }
            prom->set_exception(std::make_exception_ptr(e_find)); 
        });

    return fut; 
}

// Теперь явно указываем std::future
std::future<void> PgDbService::insertMessage(const MessageData& message_data) {
    auto prom = std::make_shared<std::promise<void>>();
    std::future<void> fut = prom->get_future();

    if (!dbClient_) {
        return handleDbClientNotAvailable(prom);
    }

    auto messagesMapper = Mapper<Messages>(dbClient_);
    Messages newMessage;

    newMessage.setUserId(message_data.user_id);
    newMessage.setCommandText(message_data.command_text); 
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
// Теперь явно указываем std::future
std::future<void> PgDbService::insertSubscription(const SubscriptionData& sub_data) {
    auto prom = std::make_shared<std::promise<void>>();
    std::future<void> fut = prom->get_future(); 

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
            prom->set_value(); 
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
            prom->set_exception(std::make_exception_ptr(e_insert)); 
        });

    return fut;
}

// Alerts
// Теперь явно указываем std::future
std::future<void> PgDbService::insertAlert(const AlertData& alert_data) {
    auto prom = std::make_shared<std::promise<void>>();
    std::future<void> fut = prom->get_future(); 

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

// Теперь явно указываем std::future
std::future<void> PgDbService::upsertWeatherCache(const WeatherCacheData& cache_data) {
    auto prom = std::make_shared<std::promise<void>>();
    std::future<void> fut = prom->get_future();

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