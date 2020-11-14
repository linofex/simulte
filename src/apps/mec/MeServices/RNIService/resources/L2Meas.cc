#include "../../RNIService/resources/L2Meas.h"
#include "CellUEInfo.h"
#include "corenetwork/lteCellInfo/LteCellInfo.h"


L2Meas::L2Meas() {}

L2Meas::L2Meas(std::vector<cModule*>& eNodeBs) {
	std::vector<cModule*>::iterator it = eNodeBs.begin();
	for(; it != eNodeBs.end() ; ++it){
	    LteCellInfo *cellInfo = check_and_cast<LteCellInfo *>((*it)->getSubmodule("cellInfo"));
		eNodeBs_.insert(std::pair<MacCellId, CellInfo>(cellInfo->getMacCellId(), CellInfo(*it)));
	}
}

void L2Meas::addEnodeB(std::vector<cModule*>& eNodeBs) {
    std::vector<cModule*>::iterator it = eNodeBs.begin();
        for(; it != eNodeBs.end() ; ++it){
            LteCellInfo *cellInfo = check_and_cast<LteCellInfo *>((*it)->getSubmodule("cellInfo"));
            eNodeBs_.insert(std::pair<MacCellId, CellInfo>(cellInfo->getMacCellId(), CellInfo(*it)));
        }
}

void L2Meas::addEnodeB(cModule* eNodeB) {
    LteCellInfo *cellInfo = check_and_cast<LteCellInfo *>(eNodeB->getSubmodule("cellInfo"));
    eNodeBs_.insert(std::pair<MacCellId, CellInfo>(cellInfo->getMacCellId(), CellInfo(eNodeB)));
}


L2Meas::~L2Meas() {}


nlohmann::ordered_json L2Meas::toJson() const {
	nlohmann::ordered_json val ;
	nlohmann::ordered_json l2Meas;
	nlohmann::ordered_json cellArray;
	nlohmann::ordered_json ueArray;

	if (timestamp_.isValid())
	{
//		timestamp_.setSeconds();
		val["timestamp"] = timestamp_.toJson();
	}



	std::map<MacCellId, CellInfo>::const_iterator it = eNodeBs_.begin();
	for(; it != eNodeBs_.end() ; ++it){
	    UeStatsCollectorMap *ueMap = it->second.getCollectorMap();
        UeStatsCollectorMap::const_iterator uit = ueMap->begin();
        UeStatsCollectorMap::const_iterator end = ueMap->end();
        for(; uit != end ; ++uit)
        {
            CellUEInfo cellUeInfo = CellUEInfo(uit->second, it->second.getEcgi());
            ueArray.push_back(cellUeInfo.toJson());
        }
		cellArray.push_back(it->second.toJson());
	}

	if(cellArray.size() > 1){
		val["cellInfo"] = cellArray;
    }
	else if(cellArray.size() == 1){
		val["cellInfo"] = cellArray[0];
	}

	if(ueArray.size() > 1){
		val["CellUEInfo"] = ueArray;
    }
	else if(ueArray.size() == 1){
		val["CellUEInfo"] = ueArray[0];
	}

	
	l2Meas["L2Meas"] = val;
	return l2Meas;
}

//
nlohmann::ordered_json L2Meas::toJson(std::vector<std::string>& uesID) const {
//	nlohmann::ordered_json val = nlohmann::ordered_json::object();
//
//	val["timestamp"] = timestamp_.toJson();
//
//	nlohmann::ordered_json jsonArray;
//
//	std::map<NodeId, CellInfo>::const_iterator it = eNodeBs_.begin();
//	for(; it != eNodeBs_.end() ; ++it){
//		jsonArray.push_back(it->second->toJson(uesID));
//	}
//
//	if(jsonArray.size() > 0){
//		val["cellInfo"] = jsonArray;
//    }
//	return val;
}
//
nlohmann::ordered_json L2Meas::toJson(std::vector<MacNodeId>& cellsID) const {
	nlohmann::ordered_json val;

//	val["timestamp"] = timestamp_.toJson();
	
	nlohmann::ordered_json jsonArray;

	std::vector<MacCellId>::const_iterator it = cellsID.begin();
	for(; it != cellsID.end() ; ++it){
		if(eNodeBs_.find(*it) != eNodeBs_.end())
			jsonArray.push_back(eNodeBs_.at(*it).toJson());
	}

	if(jsonArray.size() > 0){
		val["cellInfo"] = jsonArray;
    }
	return val;
}
//
nlohmann::ordered_json L2Meas::toJson(std::vector<MacNodeId>& cellsID, std::vector<MacNodeId>& uesID) const {
//	nlohmann::ordered_json val ;
////	val["timestamp"] = timestamp_.toJson();
//
//	nlohmann::ordered_json jsonArray;
//
//	std::vector<NodeId>::const_iterator it = cellsID.begin();
//	for(; it != cellsID.end() ; ++it){
//		if(eNodeBs_.find(*it) != eNodeBs_.end())
//			jsonArray.push_back(eNodeBs_.at(*it).toJson(uesID));
//	}
//
//	if(jsonArray.size() > 0){
//		val["cellInfo"] = jsonArray;
//    }
//	return val;
}
//
//
//
//









	
