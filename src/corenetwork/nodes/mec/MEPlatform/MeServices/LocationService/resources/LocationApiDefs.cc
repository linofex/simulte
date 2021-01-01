
#include "corenetwork/nodes/mec/MEPlatform/MeServices/LocationService/resources/LocationApiDefs.h"

namespace LocationUtils
{


    inet::Coord getCoordinates(const MacNodeId id)
    {
        LteBinder* binder = getBinder();
        OmnetId omnetId = binder->getOmnetId(id);
        if(omnetId == 0)
            return inet::Coord::ZERO; // or throw exception?
        omnetpp::cModule* module = getSimulation()->getModule(omnetId);
        inet::MovingMobilityBase *mobility_ = check_and_cast<inet::MovingMobilityBase *>(module->getSubmodule("mobility"));
        return mobility_->getCurrentPosition();
    }

}
