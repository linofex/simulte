#include "../../LocationService/resources/LocationResource.h"
#include "corenetwork/lteCellInfo/LteCellInfo.h"
#include "inet/mobility/base/MovingMobilityBase.h"

#include "apps/mec/MeServices/LocationService/resources/UserInfo.h"
#include "corenetwork/binder/LteBinder.h"
LocationResource::LocationResource() {
    binder_ = nullptr;
}

LocationResource::LocationResource(std::string& baseUri, std::vector<cModule*>& eNodeBs, LteBinder *binder) {
    binder_ = binder;
    baseUri_ = baseUri;
	std::vector<cModule*>::iterator it = eNodeBs.begin();
	for(; it != eNodeBs.end() ; ++it){
	    LteCellInfo * cellInfo = check_and_cast<LteCellInfo *>((*it)->getSubmodule("cellInfo"));
		eNodeBs_.insert(std::pair<MacCellId, LteCellInfo *>(cellInfo->getMacCellId(), cellInfo));
	}
}

void LocationResource::addEnodeB(std::vector<cModule*>& eNodeBs) {
    std::vector<cModule*>::iterator it = eNodeBs.begin();
        for(; it != eNodeBs.end() ; ++it){
            LteCellInfo * cellInfo = check_and_cast<LteCellInfo *>((*it)->getSubmodule("cellInfo"));
                    eNodeBs_.insert(std::pair<MacCellId, LteCellInfo *>(cellInfo->getMacCellId(), cellInfo));
        }
}

void LocationResource::addEnodeB(cModule* eNodeB) {

    LteCellInfo * cellInfo = check_and_cast<LteCellInfo *>(eNodeB->getSubmodule("cellInfo"));
    eNodeBs_.insert(std::pair<MacCellId, LteCellInfo *>(cellInfo->getMacCellId(), cellInfo));
    EV << "LocationResource::addEnodeB with cellId: "<< cellInfo->getMacCellId() << endl;
}

void LocationResource::addBinder(LteBinder *binder)
{
    binder_= binder;
}

void LocationResource::setBaseUri(const std::string& baseUri)
{
    baseUri_ = baseUri;

}

LocationResource::~LocationResource(){}


nlohmann::ordered_json LocationResource::toJson() const {
	nlohmann::ordered_json val ;
	nlohmann::ordered_json userList;
	nlohmann::ordered_json ueArray;
	std::map<MacCellId, LteCellInfo *>::const_iterator it = eNodeBs_.begin();
	const std::map<MacNodeId, inet::Coord>* uePositionList;
	for(; it != eNodeBs_.end() ; ++it){
	    uePositionList = it->second->getUePositionList();
	    std::map<MacNodeId, inet::Coord>::const_iterator pit = uePositionList->begin();
	    std::map<MacNodeId, inet::Coord>::const_iterator end = uePositionList->end();
	    for(; pit != end ; ++pit)
	    {
	        std::string ipAddress = binder_->getIPv4Address(pit->first).str();
	        std::string refUrl = baseUri_ + "?address=acr:" + ipAddress;
	        inet::Coord  speed = getSpeed(pit->first);
//	        UserInfo ueInfo = UserInfo(pit->second, speed , ipAddress, it->first, refUrl);
	        UserInfo ueInfo = UserInfo(getCoords(pit->first), speed , ipAddress, it->first, refUrl);

	        ueArray.push_back(ueInfo.toJson());
	    }
	}

	if(ueArray.size() > 1){
		val["user"] = ueArray;
    }
	else if(ueArray.size() == 1){
		val["user"] = ueArray[0];
	}

	userList["userList"] = val;

return userList;
}

//
nlohmann::ordered_json LocationResource::toJsonUe(std::vector<IPv4Address>& uesID) const {
	nlohmann::ordered_json val ;
	nlohmann::ordered_json ueArray;

	std::vector<IPv4Address>::const_iterator uit = uesID.begin();
	std::map<MacCellId, LteCellInfo*>::const_iterator eit;
	std::map<MacNodeId, inet::Coord>::const_iterator pit;
	const std::map<MacNodeId, inet::Coord>* uePositionList;
	bool found = false;
	for(; uit != uesID.end() ; ++uit){
	    MacNodeId nodeId = binder_->getMacNodeId(*uit);
	    found = false;
	    eit = eNodeBs_.begin();
	    for(; eit != eNodeBs_.end() ; ++eit){
	        uePositionList = eit->second->getUePositionList();
	        pit = uePositionList->find(nodeId);
	        if(pit != uePositionList->end())
	        {
	            std::string refUrl = baseUri_ + "?address=acr:" + (*uit).str();
	            inet::Coord  speed = this->getSpeed(nodeId);
//                UserInfo ueInfo = UserInfo(pit->second, speed, (*uit).str(), eit->first, refUrl);
                UserInfo ueInfo = UserInfo(getCoords(pit->first), speed, (*uit).str(), eit->first, refUrl);

                ueArray.push_back(ueInfo.toJson());
                found = true;
                break; // next ue id
	        }
        }
        if(!found)
        {
            std::string notFound = "Address: " + (*uit).str() + " Not found.";
            ueArray.push_back(notFound);
        }
	}
	if(ueArray.size() > 1){
        val["userInfo"] = ueArray;
	}
    else if(ueArray.size() == 1){
        val["userInfo"] = ueArray[0];
    }
	return val;
}


//
nlohmann::ordered_json LocationResource::toJsonCell(std::vector<MacCellId>& cellsID) const
{
    nlohmann::ordered_json val ;
    nlohmann::ordered_json LocationResource;
    nlohmann::ordered_json ueArray;

    std::vector<MacCellId>::const_iterator cid =  cellsID.begin();
    std::map<MacCellId, LteCellInfo *>::const_iterator it;
    const std::map<MacNodeId, inet::Coord>* uePositionList;

    for(; cid != cellsID.end() ; ++cid){
        it = eNodeBs_.find(*cid);
        if(it != eNodeBs_.end()){
            uePositionList = it->second->getUePositionList();
            std::map<MacNodeId, inet::Coord>::const_iterator pit = uePositionList->begin();
            std::map<MacNodeId, inet::Coord>::const_iterator end = uePositionList->end();
            for(; pit != end ; ++pit)
            {
                std::string ipAddress = binder_->getIPv4Address(pit->first).str();
                std::string refUrl = baseUri_ + "?address=acr:" + ipAddress;
                inet::Coord  speed = getSpeed(pit->first);
//                UserInfo ueInfo = UserInfo(pit->second, speed , ipAddress, it->first, refUrl);
                UserInfo ueInfo = UserInfo(getCoords(pit->first), speed , ipAddress, it->first, refUrl);

                ueArray.push_back(ueInfo.toJson());
            }
        }
        else
        {
            std::string notFound = "AccessPointId: " + std::to_string(*cid) + " Not found.";
            val["Not Found"] = notFound;
        }

    }

    if(ueArray.size() > 1){
        val["user"] = ueArray;
    }
    else if(ueArray.size() == 1){
        val["user"] = ueArray[0];
    }

    LocationResource["userList"] = val;
    return LocationResource;
}
////
nlohmann::ordered_json LocationResource::toJson(std::vector<MacCellId>& cellsID, std::vector<IPv4Address>& uesID) const
{
//	nlohmann::ordered_json val ;
//    nlohmann::ordered_json LocationResource;
//	val["cellInfo"] = toJsonCell(cellsID)["LocationResource"]["cellInfo"];
//	val["CellUEInfo"] = toJsonUe(uesID)["LocationResource"]["CellUEInfo"];
//	LocationResource["LocationResource"] = val;
//	return LocationResource;

	
}


inet::Coord LocationResource::getSpeed(const MacNodeId id) const
{
    // binder già c'è nella risorsa...
    // bineder_
    LteBinder* temp = getBinder();
    OmnetId omnetId = temp->getOmnetId(id);
    omnetpp::cModule* module = getSimulation()->getModule(omnetId);
    inet::MovingMobilityBase *mobility_ = check_and_cast<inet::MovingMobilityBase *>(module->getSubmodule("mobility"));
    return mobility_->getCurrentSpeed();
}


inet::Coord LocationResource::getCoords(const MacNodeId id) const
{
    LteBinder* temp = getBinder();
    OmnetId omnetId = temp->getOmnetId(id);
    omnetpp::cModule* module = getSimulation()->getModule(omnetId);
    inet::MovingMobilityBase *mobility_ = check_and_cast<inet::MovingMobilityBase *>(module->getSubmodule("mobility"));
    return mobility_->getCurrentPosition();
}




