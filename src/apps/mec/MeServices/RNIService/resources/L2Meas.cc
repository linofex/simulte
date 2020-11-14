#include "../../RNIService/resources/L2Meas.h"

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


	if (timestamp_.isValid())
	{
//		timestamp_.setSeconds();
		val["timestamp"] = timestamp_.toJson();
	}
	nlohmann::ordered_json jsonArray;

	std::map<MacCellId, CellInfo>::const_iterator it = eNodeBs_.begin();
	for(; it != eNodeBs_.end() ; ++it){
		jsonArray.push_back(it->second.toJson());
	}

	if(jsonArray.size() > 1){
		val["cellInfo"] = jsonArray;
    }
	else if(jsonArray.size() == 1){
		val["cellInfo"] = jsonArray[0];
	}
	//I have to add all the users
	
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









	
