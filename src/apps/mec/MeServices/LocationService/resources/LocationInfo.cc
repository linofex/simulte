/*
 * LocationInfo.cc
 *
 *  Created on: Dec 5, 2020
 *      Author: linofex
 */





#include "apps/mec/MeServices/LocationService/resources/LocationInfo.h"

LocationInfo::LocationInfo()
{
    coordinates_ = inet::Coord(0.,0.,0.);

}
LocationInfo::LocationInfo(inet::Coord coordinates)
{
    coordinates_ = coordinates;
}
LocationInfo::~LocationInfo(){}

nlohmann::ordered_json LocationInfo::toJson() const
{
    nlohmann::ordered_json val;
    val["x"] = coordinates_.x;
    val["y"] = coordinates_.y;
    val["z"] = coordinates_.z;

 return val;
}
