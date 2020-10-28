#include "L2Meas.h"
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


nlohmann::json L2Meas::toJson() const {
	nlohmann::json val = nlohmann::json::object();

//	val["timestamp"] = timestamp_.toJson();

	nlohmann::json jsonArray;

	std::map<MacCellId, CellInfo>::const_iterator it = eNodeBs_.begin();
	for(; it != eNodeBs_.end() ; ++it){
		jsonArray.push_back(it->second.toJson());
	}

	if(jsonArray.size() > 0){
		val["cellInfo"] = jsonArray;
    }

	return val;
}

//
//nlohmann::json L2Meas::toJson(std::vector<Ipv4>& uesID) const {
//	nlohmann::json val = nlohmann::json::object();
//
//	val["timestamp"] = timestamp_.toJson();
//
//	nlohmann::json jsonArray;
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
//}
//
nlohmann::json L2Meas::toJson(std::vector<MacCellId>& cellsID) const {
	nlohmann::json val = nlohmann::json::object();

//	val["timestamp"] = timestamp_.toJson();
	
	nlohmann::json jsonArray;

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
//nlohmann::json L2Meas::toJson(std::vector<NodeId>& cellsID, std::vector<Ipv4>& uesID) const {
//	nlohmann::json val = nlohmann::json::object();
//
////	val["timestamp"] = timestamp_.toJson();
//
//	nlohmann::json jsonArray;
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
//}
//
//
//
//









	
