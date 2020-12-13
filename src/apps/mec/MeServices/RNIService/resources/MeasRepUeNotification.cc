#include "apps/mec/MeServices/RNIService/resources/MeasRepUeNotification.h"
#include "corenetwork/statsCollector/UeStatsCollector.h"
#include "corenetwork/statsCollector/EnodeBStatsCollector.h"

#include "CellUEInfo.h"

MeasRepUeNotification::MeasRepUeNotification() {}

MeasRepUeNotification::MeasRepUeNotification(std::vector<cModule*>& eNodeBs) {
	std::vector<cModule*>::iterator it = eNodeBs.begin();
	for(; it != eNodeBs.end() ; ++it){
		EnodeBStatsCollector * collector = check_and_cast<EnodeBStatsCollector *>((*it)->getSubmodule("collector"));
		eNodeBs_.insert(std::pair<MacCellId, EnodeBStatsCollector *>(collector->getCellId(), collector));
	}
}

void MeasRepUeNotification::addEnodeB(std::vector<cModule*>& eNodeBs) {
    std::vector<cModule*>::iterator it = eNodeBs.begin();
        for(; it != eNodeBs.end() ; ++it){
			EnodeBStatsCollector * collector = check_and_cast<EnodeBStatsCollector *>((*it)->getSubmodule("collector"));
            eNodeBs_.insert(std::pair<MacCellId, EnodeBStatsCollector*>(collector->getCellId(), collector));
        }
}

void MeasRepUeNotification::addEnodeB(cModule* eNodeB) {
    EnodeBStatsCollector * collector = check_and_cast<EnodeBStatsCollector *>(eNodeB->getSubmodule("collector"));
	eNodeBs_.insert(std::pair<MacCellId, EnodeBStatsCollector *>(collector->getCellId(), collector));
}

bool MeasRepUeNotification::startNotification(std::vector<MacNodeId>& cellsID, std::vector<MacNodeId>& uesID, Trigger trigger)
{
    // Even it is a vector, thhis first implementation manages only vectors with size == 1
    //check if the node id is in the cell and
    MacCellId cellId = cellsID.at(0);
    MacNodeId ue = uesID.at(0);
    std::map<MacCellId, EnodeBStatsCollector*>::const_iterator it = eNodeBs_.find(cellId);
    if(it != eNodeBs_.end())
    {
        if(it->second->hasUeCollector(ue))
        {
            UeStatsCollector *ueColl = it->second->getUeCollector(ue);
            //ueColl->startNotification(trigger);
            return true;
        }
    }
    return false;
}


MeasRepUeNotification::~MeasRepUeNotification() {}


nlohmann::ordered_json MeasRepUeNotification::toJson() const {
//	nlohmann::ordered_json val ;
//	nlohmann::ordered_json MeasRepUeNotification;
//	nlohmann::ordered_json cellArray;
//	nlohmann::ordered_json ueArray;
//
//	if (timestamp_.isValid())
//	{
////		timestamp_.setSeconds();
//		val["timestamp"] = timestamp_.toJson();
//	}
//
//	std::map<MacCellId, EnodeBStatsCollector *>::const_iterator it = eNodeBs_.begin();
//	for(; it != eNodeBs_.end() ; ++it){
//	    UeStatsCollectorMap *ueMap = it->second->getCollectorMap();
//        UeStatsCollectorMap::const_iterator uit = ueMap->begin();
//        UeStatsCollectorMap::const_iterator end = ueMap->end();
//        for(; uit != end ; ++uit)
//        {
//            CellUEInfo cellUeInfo = CellUEInfo(uit->second, it->second->getEcgi());
//            ueArray.push_back(cellUeInfo.toJson());
//        }
//		CellInfo cellInfo = CellInfo(it->second);
//		cellArray.push_back(cellInfo.toJson());
//	}
//
//	if(cellArray.size() > 1){
//		val["cellInfo"] = cellArray;
//    }
//	else if(cellArray.size() == 1){
//		val["cellInfo"] = cellArray[0];
//	}
//
//	if(ueArray.size() > 1){
//		val["CellUEInfo"] = ueArray;
//    }
//	else if(ueArray.size() == 1){
//		val["CellUEInfo"] = ueArray[0];
//	}
//
//	MeasRepUeNotification["MeasRepUeNotification"] = val;
//	return MeasRepUeNotification;
}

//
nlohmann::ordered_json MeasRepUeNotification::toJsonUe(std::vector<MacNodeId>& uesID) const {
	nlohmann::ordered_json val ;
	nlohmann::ordered_json MeasRepUeNotification;
	nlohmann::ordered_json ueArray;

	if (timestamp_.isValid())
	{
//		timestamp_.setSeconds();
		val["timestamp"] = timestamp_.toJson();
	}


	std::vector<MacNodeId>::const_iterator uit = uesID.begin();
	std::map<MacCellId, EnodeBStatsCollector*>::const_iterator eit;
	bool found = false;
	for(; uit != uesID.end() ; ++uit){
	    found = false;
	    eit = eNodeBs_.begin();
	    for(; eit != eNodeBs_.end() ; ++eit){
            if(eit->second->hasUeCollector(*uit))
            {
                UeStatsCollector *ueColl = eit->second->getUeCollector(*uit);
                CellUEInfo cellUeInfo = CellUEInfo(ueColl, eit->second->getEcgi());
                ueArray.push_back(cellUeInfo.toJson());
                found = true;
                break; // next ue id
            }
        }
        if(!found)
        {

        }
	}

	if(ueArray.size() > 1){
        val["CellUEInfo"] = ueArray;
	}
    else if(ueArray.size() == 1){
        val["CellUEInfo"] = ueArray[0];
    }

	MeasRepUeNotification["MeasRepUeNotification"] = val;
	return MeasRepUeNotification;
}


//
nlohmann::ordered_json MeasRepUeNotification::toJsonCell(std::vector<MacCellId>& cellsID) const
{
//    nlohmann::ordered_json val ;
//    nlohmann::ordered_json MeasRepUeNotification;
//    nlohmann::ordered_json cellArray;
//
//        if (timestamp_.isValid())
//        {
//    //      timestamp_.setSeconds();
//            val["timestamp"] = timestamp_.toJson();
//        }
//
//        std::vector<MacCellId>::const_iterator cid =  cellsID.begin();
//        std::map<MacCellId, EnodeBStatsCollector *>::const_iterator it;
//        for(; cid != cellsID.end() ; ++cid){
//            it = eNodeBs_.find(*cid);
//            if(it != eNodeBs_.end()){
//                CellInfo cellInfo = CellInfo(it->second);
//                cellArray.push_back(cellInfo.toJson());
//            }
//        }
//
//        if(cellArray.size() > 1){
//            val["cellInfo"] = cellArray;
//        }
//        else if(cellArray.size() == 1){
//            val["cellInfo"] = cellArray[0];
//        }
//
//        MeasRepUeNotification["MeasRepUeNotification"] = val;
//        return MeasRepUeNotification;

}
////
nlohmann::ordered_json MeasRepUeNotification::toJson(std::vector<MacCellId>& cellsID, std::vector<MacNodeId>& uesID) const
{
	nlohmann::ordered_json val ;
    nlohmann::ordered_json MeasRepUeNotification;
	val["cellInfo"] = toJsonCell(cellsID)["MeasRepUeNotification"]["cellInfo"];
	val["CellUEInfo"] = toJsonUe(uesID)["MeasRepUeNotification"]["CellUEInfo"];
	MeasRepUeNotification["MeasRepUeNotification"] = val;
	return MeasRepUeNotification;

	
}
