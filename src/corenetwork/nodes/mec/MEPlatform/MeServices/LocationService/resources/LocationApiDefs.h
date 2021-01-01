/*
 * LocationApiDefs.h
 *
 *  Created on: Dec 28, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_LOCATIONAPIDEFS_H_
#define APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_LOCATIONAPIDEFS_H_

#include "corenetwork/binder/LteBinder.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/mobility/base/MovingMobilityBase.h"


namespace LocationUtils{

    /* From: RESTful Network APIforTerminal Location
    * section 5.2.3.1
    */
    enum EnteringLeavingCriteria {Entering, Leaving};
    /* From: RESTful Network APIforTerminal Location
    * section 5.2.3.2
    */
    enum DistanceCriteria {AllWithinDistance, AnyWithinDistance, AllBeyondDistance, AnyBeyondDistance};

    inet::Coord getCoordinates(const MacNodeId id);

}



#endif /* APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_LOCATIONAPIDEFS_H_ */
