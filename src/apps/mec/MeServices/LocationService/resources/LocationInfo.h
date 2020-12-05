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
        LocationInfo(inet::Coord coordinates);
        virtual ~LocationInfo();
        nlohmann::ordered_json toJson() const override;

    private:
        inet::Coord coordinates_;
};


#endif /* APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_LOCATIONINFO_H_ */
