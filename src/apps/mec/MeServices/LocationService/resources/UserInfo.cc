/*
 * UserInfo.cc
 *
 *  Created on: Dec 5, 2020
 *      Author: linofex
 */



#include "apps/mec/MeServices/LocationService/resources/UserInfo.h"

UserInfo::UserInfo():timestamp_(), locationInfo_(){}
UserInfo::UserInfo(const LocationInfo& location, const std::string& address, MacCellId accessPointId, const std::string& resourceUrl): locationInfo_(location)
{
    address_ = address;
    accessPointId_ = accessPointId;
    resourceUrl_ = resourceUrl;
}
UserInfo::UserInfo(const inet::Coord& location, const std::string& address, MacCellId accessPointId, const std::string& resourceUrl): locationInfo_(location)
{
    address_ = address;
    accessPointId_ = accessPointId;
    resourceUrl_ = resourceUrl;
}


nlohmann::ordered_json UserInfo::toJson() const
{
    nlohmann::ordered_json val;
    val["address"] = "acr:"+address_;
    val["accessPointId"] = accessPointId_;
    val["resourceURL"] = resourceUrl_;
    val["timeStamp"] = timestamp_.toJson();
    val["locationInfo"] = locationInfo_.toJson();
    return val;
}
