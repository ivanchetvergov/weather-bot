/**
 *
 *  Subscriptions.cc
 *  DO NOT EDIT. This file is generated by drogon_ctl
 *
 */

#include "Subscriptions.h"
#include <drogon/utils/Utilities.h>
#include <string>

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::weather_bot;

const std::string Subscriptions::Cols::_id = "\"id\"";
const std::string Subscriptions::Cols::_user_id = "\"user_id\"";
const std::string Subscriptions::Cols::_city = "\"city\"";
const std::string Subscriptions::Cols::_temp_above = "\"temp_above\"";
const std::string Subscriptions::Cols::_rain = "\"rain\"";
const std::string Subscriptions::Cols::_wind_speed_gt = "\"wind_speed_gt\"";
const std::string Subscriptions::Cols::_notify_time = "\"notify_time\"";
const std::string Subscriptions::primaryKeyName = "id";
const bool Subscriptions::hasPrimaryKey = true;
const std::string Subscriptions::tableName = "\"subscriptions\"";

const std::vector<typename Subscriptions::MetaData> Subscriptions::metaData_={
{"id","int32_t","integer",4,1,1,1},
{"user_id","int64_t","bigint",8,0,0,1},
{"city","std::string","text",0,0,0,1},
{"temp_above","double","double precision",8,0,0,0},
{"rain","bool","boolean",1,0,0,0},
{"wind_speed_gt","double","double precision",8,0,0,0},
{"notify_time","std::string","time without time zone",0,0,0,0}
};
const std::string &Subscriptions::getColumnName(size_t index) noexcept(false)
{
    assert(index < metaData_.size());
    return metaData_[index].colName_;
}
Subscriptions::Subscriptions(const Row &r, const ssize_t indexOffset) noexcept
{
    if(indexOffset < 0)
    {
        if(!r["id"].isNull())
        {
            id_=std::make_shared<int32_t>(r["id"].as<int32_t>());
        }
        if(!r["user_id"].isNull())
        {
            userId_=std::make_shared<int64_t>(r["user_id"].as<int64_t>());
        }
        if(!r["city"].isNull())
        {
            city_=std::make_shared<std::string>(r["city"].as<std::string>());
        }
        if(!r["temp_above"].isNull())
        {
            tempAbove_=std::make_shared<double>(r["temp_above"].as<double>());
        }
        if(!r["rain"].isNull())
        {
            rain_=std::make_shared<bool>(r["rain"].as<bool>());
        }
        if(!r["wind_speed_gt"].isNull())
        {
            windSpeedGt_=std::make_shared<double>(r["wind_speed_gt"].as<double>());
        }
        if(!r["notify_time"].isNull())
        {
            notifyTime_=std::make_shared<std::string>(r["notify_time"].as<std::string>());
        }
    }
    else
    {
        size_t offset = (size_t)indexOffset;
        if(offset + 7 > r.size())
        {
            LOG_FATAL << "Invalid SQL result for this model";
            return;
        }
        size_t index;
        index = offset + 0;
        if(!r[index].isNull())
        {
            id_=std::make_shared<int32_t>(r[index].as<int32_t>());
        }
        index = offset + 1;
        if(!r[index].isNull())
        {
            userId_=std::make_shared<int64_t>(r[index].as<int64_t>());
        }
        index = offset + 2;
        if(!r[index].isNull())
        {
            city_=std::make_shared<std::string>(r[index].as<std::string>());
        }
        index = offset + 3;
        if(!r[index].isNull())
        {
            tempAbove_=std::make_shared<double>(r[index].as<double>());
        }
        index = offset + 4;
        if(!r[index].isNull())
        {
            rain_=std::make_shared<bool>(r[index].as<bool>());
        }
        index = offset + 5;
        if(!r[index].isNull())
        {
            windSpeedGt_=std::make_shared<double>(r[index].as<double>());
        }
        index = offset + 6;
        if(!r[index].isNull())
        {
            notifyTime_=std::make_shared<std::string>(r[index].as<std::string>());
        }
    }

}

Subscriptions::Subscriptions(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false)
{
    if(pMasqueradingVector.size() != 7)
    {
        LOG_ERROR << "Bad masquerading vector";
        return;
    }
    if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
    {
        dirtyFlag_[0] = true;
        if(!pJson[pMasqueradingVector[0]].isNull())
        {
            id_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[0]].asInt64());
        }
    }
    if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
    {
        dirtyFlag_[1] = true;
        if(!pJson[pMasqueradingVector[1]].isNull())
        {
            userId_=std::make_shared<int64_t>((int64_t)pJson[pMasqueradingVector[1]].asInt64());
        }
    }
    if(!pMasqueradingVector[2].empty() && pJson.isMember(pMasqueradingVector[2]))
    {
        dirtyFlag_[2] = true;
        if(!pJson[pMasqueradingVector[2]].isNull())
        {
            city_=std::make_shared<std::string>(pJson[pMasqueradingVector[2]].asString());
        }
    }
    if(!pMasqueradingVector[3].empty() && pJson.isMember(pMasqueradingVector[3]))
    {
        dirtyFlag_[3] = true;
        if(!pJson[pMasqueradingVector[3]].isNull())
        {
            tempAbove_=std::make_shared<double>(pJson[pMasqueradingVector[3]].asDouble());
        }
    }
    if(!pMasqueradingVector[4].empty() && pJson.isMember(pMasqueradingVector[4]))
    {
        dirtyFlag_[4] = true;
        if(!pJson[pMasqueradingVector[4]].isNull())
        {
            rain_=std::make_shared<bool>(pJson[pMasqueradingVector[4]].asBool());
        }
    }
    if(!pMasqueradingVector[5].empty() && pJson.isMember(pMasqueradingVector[5]))
    {
        dirtyFlag_[5] = true;
        if(!pJson[pMasqueradingVector[5]].isNull())
        {
            windSpeedGt_=std::make_shared<double>(pJson[pMasqueradingVector[5]].asDouble());
        }
    }
    if(!pMasqueradingVector[6].empty() && pJson.isMember(pMasqueradingVector[6]))
    {
        dirtyFlag_[6] = true;
        if(!pJson[pMasqueradingVector[6]].isNull())
        {
            notifyTime_=std::make_shared<std::string>(pJson[pMasqueradingVector[6]].asString());
        }
    }
}

Subscriptions::Subscriptions(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("id"))
    {
        dirtyFlag_[0]=true;
        if(!pJson["id"].isNull())
        {
            id_=std::make_shared<int32_t>((int32_t)pJson["id"].asInt64());
        }
    }
    if(pJson.isMember("user_id"))
    {
        dirtyFlag_[1]=true;
        if(!pJson["user_id"].isNull())
        {
            userId_=std::make_shared<int64_t>((int64_t)pJson["user_id"].asInt64());
        }
    }
    if(pJson.isMember("city"))
    {
        dirtyFlag_[2]=true;
        if(!pJson["city"].isNull())
        {
            city_=std::make_shared<std::string>(pJson["city"].asString());
        }
    }
    if(pJson.isMember("temp_above"))
    {
        dirtyFlag_[3]=true;
        if(!pJson["temp_above"].isNull())
        {
            tempAbove_=std::make_shared<double>(pJson["temp_above"].asDouble());
        }
    }
    if(pJson.isMember("rain"))
    {
        dirtyFlag_[4]=true;
        if(!pJson["rain"].isNull())
        {
            rain_=std::make_shared<bool>(pJson["rain"].asBool());
        }
    }
    if(pJson.isMember("wind_speed_gt"))
    {
        dirtyFlag_[5]=true;
        if(!pJson["wind_speed_gt"].isNull())
        {
            windSpeedGt_=std::make_shared<double>(pJson["wind_speed_gt"].asDouble());
        }
    }
    if(pJson.isMember("notify_time"))
    {
        dirtyFlag_[6]=true;
        if(!pJson["notify_time"].isNull())
        {
            notifyTime_=std::make_shared<std::string>(pJson["notify_time"].asString());
        }
    }
}

void Subscriptions::updateByMasqueradedJson(const Json::Value &pJson,
                                            const std::vector<std::string> &pMasqueradingVector) noexcept(false)
{
    if(pMasqueradingVector.size() != 7)
    {
        LOG_ERROR << "Bad masquerading vector";
        return;
    }
    if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
    {
        if(!pJson[pMasqueradingVector[0]].isNull())
        {
            id_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[0]].asInt64());
        }
    }
    if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
    {
        dirtyFlag_[1] = true;
        if(!pJson[pMasqueradingVector[1]].isNull())
        {
            userId_=std::make_shared<int64_t>((int64_t)pJson[pMasqueradingVector[1]].asInt64());
        }
    }
    if(!pMasqueradingVector[2].empty() && pJson.isMember(pMasqueradingVector[2]))
    {
        dirtyFlag_[2] = true;
        if(!pJson[pMasqueradingVector[2]].isNull())
        {
            city_=std::make_shared<std::string>(pJson[pMasqueradingVector[2]].asString());
        }
    }
    if(!pMasqueradingVector[3].empty() && pJson.isMember(pMasqueradingVector[3]))
    {
        dirtyFlag_[3] = true;
        if(!pJson[pMasqueradingVector[3]].isNull())
        {
            tempAbove_=std::make_shared<double>(pJson[pMasqueradingVector[3]].asDouble());
        }
    }
    if(!pMasqueradingVector[4].empty() && pJson.isMember(pMasqueradingVector[4]))
    {
        dirtyFlag_[4] = true;
        if(!pJson[pMasqueradingVector[4]].isNull())
        {
            rain_=std::make_shared<bool>(pJson[pMasqueradingVector[4]].asBool());
        }
    }
    if(!pMasqueradingVector[5].empty() && pJson.isMember(pMasqueradingVector[5]))
    {
        dirtyFlag_[5] = true;
        if(!pJson[pMasqueradingVector[5]].isNull())
        {
            windSpeedGt_=std::make_shared<double>(pJson[pMasqueradingVector[5]].asDouble());
        }
    }
    if(!pMasqueradingVector[6].empty() && pJson.isMember(pMasqueradingVector[6]))
    {
        dirtyFlag_[6] = true;
        if(!pJson[pMasqueradingVector[6]].isNull())
        {
            notifyTime_=std::make_shared<std::string>(pJson[pMasqueradingVector[6]].asString());
        }
    }
}

void Subscriptions::updateByJson(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("id"))
    {
        if(!pJson["id"].isNull())
        {
            id_=std::make_shared<int32_t>((int32_t)pJson["id"].asInt64());
        }
    }
    if(pJson.isMember("user_id"))
    {
        dirtyFlag_[1] = true;
        if(!pJson["user_id"].isNull())
        {
            userId_=std::make_shared<int64_t>((int64_t)pJson["user_id"].asInt64());
        }
    }
    if(pJson.isMember("city"))
    {
        dirtyFlag_[2] = true;
        if(!pJson["city"].isNull())
        {
            city_=std::make_shared<std::string>(pJson["city"].asString());
        }
    }
    if(pJson.isMember("temp_above"))
    {
        dirtyFlag_[3] = true;
        if(!pJson["temp_above"].isNull())
        {
            tempAbove_=std::make_shared<double>(pJson["temp_above"].asDouble());
        }
    }
    if(pJson.isMember("rain"))
    {
        dirtyFlag_[4] = true;
        if(!pJson["rain"].isNull())
        {
            rain_=std::make_shared<bool>(pJson["rain"].asBool());
        }
    }
    if(pJson.isMember("wind_speed_gt"))
    {
        dirtyFlag_[5] = true;
        if(!pJson["wind_speed_gt"].isNull())
        {
            windSpeedGt_=std::make_shared<double>(pJson["wind_speed_gt"].asDouble());
        }
    }
    if(pJson.isMember("notify_time"))
    {
        dirtyFlag_[6] = true;
        if(!pJson["notify_time"].isNull())
        {
            notifyTime_=std::make_shared<std::string>(pJson["notify_time"].asString());
        }
    }
}

const int32_t &Subscriptions::getValueOfId() const noexcept
{
    static const int32_t defaultValue = int32_t();
    if(id_)
        return *id_;
    return defaultValue;
}
const std::shared_ptr<int32_t> &Subscriptions::getId() const noexcept
{
    return id_;
}
void Subscriptions::setId(const int32_t &pId) noexcept
{
    id_ = std::make_shared<int32_t>(pId);
    dirtyFlag_[0] = true;
}
const typename Subscriptions::PrimaryKeyType & Subscriptions::getPrimaryKey() const
{
    assert(id_);
    return *id_;
}

const int64_t &Subscriptions::getValueOfUserId() const noexcept
{
    static const int64_t defaultValue = int64_t();
    if(userId_)
        return *userId_;
    return defaultValue;
}
const std::shared_ptr<int64_t> &Subscriptions::getUserId() const noexcept
{
    return userId_;
}
void Subscriptions::setUserId(const int64_t &pUserId) noexcept
{
    userId_ = std::make_shared<int64_t>(pUserId);
    dirtyFlag_[1] = true;
}

const std::string &Subscriptions::getValueOfCity() const noexcept
{
    static const std::string defaultValue = std::string();
    if(city_)
        return *city_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Subscriptions::getCity() const noexcept
{
    return city_;
}
void Subscriptions::setCity(const std::string &pCity) noexcept
{
    city_ = std::make_shared<std::string>(pCity);
    dirtyFlag_[2] = true;
}
void Subscriptions::setCity(std::string &&pCity) noexcept
{
    city_ = std::make_shared<std::string>(std::move(pCity));
    dirtyFlag_[2] = true;
}

const double &Subscriptions::getValueOfTempAbove() const noexcept
{
    static const double defaultValue = double();
    if(tempAbove_)
        return *tempAbove_;
    return defaultValue;
}
const std::shared_ptr<double> &Subscriptions::getTempAbove() const noexcept
{
    return tempAbove_;
}
void Subscriptions::setTempAbove(const double &pTempAbove) noexcept
{
    tempAbove_ = std::make_shared<double>(pTempAbove);
    dirtyFlag_[3] = true;
}
void Subscriptions::setTempAboveToNull() noexcept
{
    tempAbove_.reset();
    dirtyFlag_[3] = true;
}

const bool &Subscriptions::getValueOfRain() const noexcept
{
    static const bool defaultValue = bool();
    if(rain_)
        return *rain_;
    return defaultValue;
}
const std::shared_ptr<bool> &Subscriptions::getRain() const noexcept
{
    return rain_;
}
void Subscriptions::setRain(const bool &pRain) noexcept
{
    rain_ = std::make_shared<bool>(pRain);
    dirtyFlag_[4] = true;
}
void Subscriptions::setRainToNull() noexcept
{
    rain_.reset();
    dirtyFlag_[4] = true;
}

const double &Subscriptions::getValueOfWindSpeedGt() const noexcept
{
    static const double defaultValue = double();
    if(windSpeedGt_)
        return *windSpeedGt_;
    return defaultValue;
}
const std::shared_ptr<double> &Subscriptions::getWindSpeedGt() const noexcept
{
    return windSpeedGt_;
}
void Subscriptions::setWindSpeedGt(const double &pWindSpeedGt) noexcept
{
    windSpeedGt_ = std::make_shared<double>(pWindSpeedGt);
    dirtyFlag_[5] = true;
}
void Subscriptions::setWindSpeedGtToNull() noexcept
{
    windSpeedGt_.reset();
    dirtyFlag_[5] = true;
}

const std::string &Subscriptions::getValueOfNotifyTime() const noexcept
{
    static const std::string defaultValue = std::string();
    if(notifyTime_)
        return *notifyTime_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Subscriptions::getNotifyTime() const noexcept
{
    return notifyTime_;
}
void Subscriptions::setNotifyTime(const std::string &pNotifyTime) noexcept
{
    notifyTime_ = std::make_shared<std::string>(pNotifyTime);
    dirtyFlag_[6] = true;
}
void Subscriptions::setNotifyTime(std::string &&pNotifyTime) noexcept
{
    notifyTime_ = std::make_shared<std::string>(std::move(pNotifyTime));
    dirtyFlag_[6] = true;
}
void Subscriptions::setNotifyTimeToNull() noexcept
{
    notifyTime_.reset();
    dirtyFlag_[6] = true;
}

void Subscriptions::updateId(const uint64_t id)
{
}

const std::vector<std::string> &Subscriptions::insertColumns() noexcept
{
    static const std::vector<std::string> inCols={
        "user_id",
        "city",
        "temp_above",
        "rain",
        "wind_speed_gt",
        "notify_time"
    };
    return inCols;
}

void Subscriptions::outputArgs(drogon::orm::internal::SqlBinder &binder) const
{
    if(dirtyFlag_[1])
    {
        if(getUserId())
        {
            binder << getValueOfUserId();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[2])
    {
        if(getCity())
        {
            binder << getValueOfCity();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[3])
    {
        if(getTempAbove())
        {
            binder << getValueOfTempAbove();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[4])
    {
        if(getRain())
        {
            binder << getValueOfRain();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[5])
    {
        if(getWindSpeedGt())
        {
            binder << getValueOfWindSpeedGt();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[6])
    {
        if(getNotifyTime())
        {
            binder << getValueOfNotifyTime();
        }
        else
        {
            binder << nullptr;
        }
    }
}

const std::vector<std::string> Subscriptions::updateColumns() const
{
    std::vector<std::string> ret;
    if(dirtyFlag_[1])
    {
        ret.push_back(getColumnName(1));
    }
    if(dirtyFlag_[2])
    {
        ret.push_back(getColumnName(2));
    }
    if(dirtyFlag_[3])
    {
        ret.push_back(getColumnName(3));
    }
    if(dirtyFlag_[4])
    {
        ret.push_back(getColumnName(4));
    }
    if(dirtyFlag_[5])
    {
        ret.push_back(getColumnName(5));
    }
    if(dirtyFlag_[6])
    {
        ret.push_back(getColumnName(6));
    }
    return ret;
}

void Subscriptions::updateArgs(drogon::orm::internal::SqlBinder &binder) const
{
    if(dirtyFlag_[1])
    {
        if(getUserId())
        {
            binder << getValueOfUserId();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[2])
    {
        if(getCity())
        {
            binder << getValueOfCity();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[3])
    {
        if(getTempAbove())
        {
            binder << getValueOfTempAbove();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[4])
    {
        if(getRain())
        {
            binder << getValueOfRain();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[5])
    {
        if(getWindSpeedGt())
        {
            binder << getValueOfWindSpeedGt();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[6])
    {
        if(getNotifyTime())
        {
            binder << getValueOfNotifyTime();
        }
        else
        {
            binder << nullptr;
        }
    }
}
Json::Value Subscriptions::toJson() const
{
    Json::Value ret;
    if(getId())
    {
        ret["id"]=getValueOfId();
    }
    else
    {
        ret["id"]=Json::Value();
    }
    if(getUserId())
    {
        ret["user_id"]=(Json::Int64)getValueOfUserId();
    }
    else
    {
        ret["user_id"]=Json::Value();
    }
    if(getCity())
    {
        ret["city"]=getValueOfCity();
    }
    else
    {
        ret["city"]=Json::Value();
    }
    if(getTempAbove())
    {
        ret["temp_above"]=getValueOfTempAbove();
    }
    else
    {
        ret["temp_above"]=Json::Value();
    }
    if(getRain())
    {
        ret["rain"]=getValueOfRain();
    }
    else
    {
        ret["rain"]=Json::Value();
    }
    if(getWindSpeedGt())
    {
        ret["wind_speed_gt"]=getValueOfWindSpeedGt();
    }
    else
    {
        ret["wind_speed_gt"]=Json::Value();
    }
    if(getNotifyTime())
    {
        ret["notify_time"]=getValueOfNotifyTime();
    }
    else
    {
        ret["notify_time"]=Json::Value();
    }
    return ret;
}

std::string Subscriptions::toString() const
{
    return toJson().toStyledString();
}

Json::Value Subscriptions::toMasqueradedJson(
    const std::vector<std::string> &pMasqueradingVector) const
{
    Json::Value ret;
    if(pMasqueradingVector.size() == 7)
    {
        if(!pMasqueradingVector[0].empty())
        {
            if(getId())
            {
                ret[pMasqueradingVector[0]]=getValueOfId();
            }
            else
            {
                ret[pMasqueradingVector[0]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[1].empty())
        {
            if(getUserId())
            {
                ret[pMasqueradingVector[1]]=(Json::Int64)getValueOfUserId();
            }
            else
            {
                ret[pMasqueradingVector[1]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[2].empty())
        {
            if(getCity())
            {
                ret[pMasqueradingVector[2]]=getValueOfCity();
            }
            else
            {
                ret[pMasqueradingVector[2]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[3].empty())
        {
            if(getTempAbove())
            {
                ret[pMasqueradingVector[3]]=getValueOfTempAbove();
            }
            else
            {
                ret[pMasqueradingVector[3]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[4].empty())
        {
            if(getRain())
            {
                ret[pMasqueradingVector[4]]=getValueOfRain();
            }
            else
            {
                ret[pMasqueradingVector[4]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[5].empty())
        {
            if(getWindSpeedGt())
            {
                ret[pMasqueradingVector[5]]=getValueOfWindSpeedGt();
            }
            else
            {
                ret[pMasqueradingVector[5]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[6].empty())
        {
            if(getNotifyTime())
            {
                ret[pMasqueradingVector[6]]=getValueOfNotifyTime();
            }
            else
            {
                ret[pMasqueradingVector[6]]=Json::Value();
            }
        }
        return ret;
    }
    LOG_ERROR << "Masquerade failed";
    if(getId())
    {
        ret["id"]=getValueOfId();
    }
    else
    {
        ret["id"]=Json::Value();
    }
    if(getUserId())
    {
        ret["user_id"]=(Json::Int64)getValueOfUserId();
    }
    else
    {
        ret["user_id"]=Json::Value();
    }
    if(getCity())
    {
        ret["city"]=getValueOfCity();
    }
    else
    {
        ret["city"]=Json::Value();
    }
    if(getTempAbove())
    {
        ret["temp_above"]=getValueOfTempAbove();
    }
    else
    {
        ret["temp_above"]=Json::Value();
    }
    if(getRain())
    {
        ret["rain"]=getValueOfRain();
    }
    else
    {
        ret["rain"]=Json::Value();
    }
    if(getWindSpeedGt())
    {
        ret["wind_speed_gt"]=getValueOfWindSpeedGt();
    }
    else
    {
        ret["wind_speed_gt"]=Json::Value();
    }
    if(getNotifyTime())
    {
        ret["notify_time"]=getValueOfNotifyTime();
    }
    else
    {
        ret["notify_time"]=Json::Value();
    }
    return ret;
}

bool Subscriptions::validateJsonForCreation(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("id"))
    {
        if(!validJsonOfField(0, "id", pJson["id"], err, true))
            return false;
    }
    if(pJson.isMember("user_id"))
    {
        if(!validJsonOfField(1, "user_id", pJson["user_id"], err, true))
            return false;
    }
    else
    {
        err="The user_id column cannot be null";
        return false;
    }
    if(pJson.isMember("city"))
    {
        if(!validJsonOfField(2, "city", pJson["city"], err, true))
            return false;
    }
    else
    {
        err="The city column cannot be null";
        return false;
    }
    if(pJson.isMember("temp_above"))
    {
        if(!validJsonOfField(3, "temp_above", pJson["temp_above"], err, true))
            return false;
    }
    if(pJson.isMember("rain"))
    {
        if(!validJsonOfField(4, "rain", pJson["rain"], err, true))
            return false;
    }
    if(pJson.isMember("wind_speed_gt"))
    {
        if(!validJsonOfField(5, "wind_speed_gt", pJson["wind_speed_gt"], err, true))
            return false;
    }
    if(pJson.isMember("notify_time"))
    {
        if(!validJsonOfField(6, "notify_time", pJson["notify_time"], err, true))
            return false;
    }
    return true;
}
bool Subscriptions::validateMasqueradedJsonForCreation(const Json::Value &pJson,
                                                       const std::vector<std::string> &pMasqueradingVector,
                                                       std::string &err)
{
    if(pMasqueradingVector.size() != 7)
    {
        err = "Bad masquerading vector";
        return false;
    }
    try {
      if(!pMasqueradingVector[0].empty())
      {
          if(pJson.isMember(pMasqueradingVector[0]))
          {
              if(!validJsonOfField(0, pMasqueradingVector[0], pJson[pMasqueradingVector[0]], err, true))
                  return false;
          }
      }
      if(!pMasqueradingVector[1].empty())
      {
          if(pJson.isMember(pMasqueradingVector[1]))
          {
              if(!validJsonOfField(1, pMasqueradingVector[1], pJson[pMasqueradingVector[1]], err, true))
                  return false;
          }
        else
        {
            err="The " + pMasqueradingVector[1] + " column cannot be null";
            return false;
        }
      }
      if(!pMasqueradingVector[2].empty())
      {
          if(pJson.isMember(pMasqueradingVector[2]))
          {
              if(!validJsonOfField(2, pMasqueradingVector[2], pJson[pMasqueradingVector[2]], err, true))
                  return false;
          }
        else
        {
            err="The " + pMasqueradingVector[2] + " column cannot be null";
            return false;
        }
      }
      if(!pMasqueradingVector[3].empty())
      {
          if(pJson.isMember(pMasqueradingVector[3]))
          {
              if(!validJsonOfField(3, pMasqueradingVector[3], pJson[pMasqueradingVector[3]], err, true))
                  return false;
          }
      }
      if(!pMasqueradingVector[4].empty())
      {
          if(pJson.isMember(pMasqueradingVector[4]))
          {
              if(!validJsonOfField(4, pMasqueradingVector[4], pJson[pMasqueradingVector[4]], err, true))
                  return false;
          }
      }
      if(!pMasqueradingVector[5].empty())
      {
          if(pJson.isMember(pMasqueradingVector[5]))
          {
              if(!validJsonOfField(5, pMasqueradingVector[5], pJson[pMasqueradingVector[5]], err, true))
                  return false;
          }
      }
      if(!pMasqueradingVector[6].empty())
      {
          if(pJson.isMember(pMasqueradingVector[6]))
          {
              if(!validJsonOfField(6, pMasqueradingVector[6], pJson[pMasqueradingVector[6]], err, true))
                  return false;
          }
      }
    }
    catch(const Json::LogicError &e)
    {
      err = e.what();
      return false;
    }
    return true;
}
bool Subscriptions::validateJsonForUpdate(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("id"))
    {
        if(!validJsonOfField(0, "id", pJson["id"], err, false))
            return false;
    }
    else
    {
        err = "The value of primary key must be set in the json object for update";
        return false;
    }
    if(pJson.isMember("user_id"))
    {
        if(!validJsonOfField(1, "user_id", pJson["user_id"], err, false))
            return false;
    }
    if(pJson.isMember("city"))
    {
        if(!validJsonOfField(2, "city", pJson["city"], err, false))
            return false;
    }
    if(pJson.isMember("temp_above"))
    {
        if(!validJsonOfField(3, "temp_above", pJson["temp_above"], err, false))
            return false;
    }
    if(pJson.isMember("rain"))
    {
        if(!validJsonOfField(4, "rain", pJson["rain"], err, false))
            return false;
    }
    if(pJson.isMember("wind_speed_gt"))
    {
        if(!validJsonOfField(5, "wind_speed_gt", pJson["wind_speed_gt"], err, false))
            return false;
    }
    if(pJson.isMember("notify_time"))
    {
        if(!validJsonOfField(6, "notify_time", pJson["notify_time"], err, false))
            return false;
    }
    return true;
}
bool Subscriptions::validateMasqueradedJsonForUpdate(const Json::Value &pJson,
                                                     const std::vector<std::string> &pMasqueradingVector,
                                                     std::string &err)
{
    if(pMasqueradingVector.size() != 7)
    {
        err = "Bad masquerading vector";
        return false;
    }
    try {
      if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
      {
          if(!validJsonOfField(0, pMasqueradingVector[0], pJson[pMasqueradingVector[0]], err, false))
              return false;
      }
    else
    {
        err = "The value of primary key must be set in the json object for update";
        return false;
    }
      if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
      {
          if(!validJsonOfField(1, pMasqueradingVector[1], pJson[pMasqueradingVector[1]], err, false))
              return false;
      }
      if(!pMasqueradingVector[2].empty() && pJson.isMember(pMasqueradingVector[2]))
      {
          if(!validJsonOfField(2, pMasqueradingVector[2], pJson[pMasqueradingVector[2]], err, false))
              return false;
      }
      if(!pMasqueradingVector[3].empty() && pJson.isMember(pMasqueradingVector[3]))
      {
          if(!validJsonOfField(3, pMasqueradingVector[3], pJson[pMasqueradingVector[3]], err, false))
              return false;
      }
      if(!pMasqueradingVector[4].empty() && pJson.isMember(pMasqueradingVector[4]))
      {
          if(!validJsonOfField(4, pMasqueradingVector[4], pJson[pMasqueradingVector[4]], err, false))
              return false;
      }
      if(!pMasqueradingVector[5].empty() && pJson.isMember(pMasqueradingVector[5]))
      {
          if(!validJsonOfField(5, pMasqueradingVector[5], pJson[pMasqueradingVector[5]], err, false))
              return false;
      }
      if(!pMasqueradingVector[6].empty() && pJson.isMember(pMasqueradingVector[6]))
      {
          if(!validJsonOfField(6, pMasqueradingVector[6], pJson[pMasqueradingVector[6]], err, false))
              return false;
      }
    }
    catch(const Json::LogicError &e)
    {
      err = e.what();
      return false;
    }
    return true;
}
bool Subscriptions::validJsonOfField(size_t index,
                                     const std::string &fieldName,
                                     const Json::Value &pJson,
                                     std::string &err,
                                     bool isForCreation)
{
    switch(index)
    {
        case 0:
            if(pJson.isNull())
            {
                err="The " + fieldName + " column cannot be null";
                return false;
            }
            if(isForCreation)
            {
                err="The automatic primary key cannot be set";
                return false;
            }
            if(!pJson.isInt())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 1:
            if(pJson.isNull())
            {
                err="The " + fieldName + " column cannot be null";
                return false;
            }
            if(!pJson.isInt64())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 2:
            if(pJson.isNull())
            {
                err="The " + fieldName + " column cannot be null";
                return false;
            }
            if(!pJson.isString())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 3:
            if(pJson.isNull())
            {
                return true;
            }
            if(!pJson.isNumeric())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 4:
            if(pJson.isNull())
            {
                return true;
            }
            if(!pJson.isBool())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 5:
            if(pJson.isNull())
            {
                return true;
            }
            if(!pJson.isNumeric())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 6:
            if(pJson.isNull())
            {
                return true;
            }
            if(!pJson.isString())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        default:
            err="Internal error in the server";
            return false;
    }
    return true;
}
