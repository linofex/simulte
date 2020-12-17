/*
 * LocationInfo.h
 *
 *  Created on: Dec 5, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_LOCATIONINFO_H_
#define APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_LOCATIONINFO_H_

#include "apps/mec/MeServices/Resources/AttributeBase.h"
#include "inet/common/geometry/common/Coord.h"

class LocationInfo : public AttributeBase
{
    public:
        LocationInfo();
        LocationInfo(const inet::Coord& coordinates, const inet::Coord& speed);
        virtual ~LocationInfo();
        nlohmann::ordered_json toJson() const override;

    private:
        inet::Coord coordinates_;
        inet::Coord speed_;
};


#endif /* APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_LOCATIONINFO_H_ */
