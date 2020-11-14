#include "CellInfo.h"
#include "corenetwork/statsCollector/EnodeBStatsCollector.h"

CellInfo::CellInfo(){}

CellInfo::CellInfo(::omnetpp::cModule* eNodeB){
  collector_ = ::omnetpp::check_and_cast<EnodeBStatsCollector*>(eNodeB->getSubmodule("collector"));
  ecgi_.setEcgi(collector_->getEcgi());
//  ueList_ =    eNodeB->getUeListCollectors();
}

CellInfo::~CellInfo(){}


nlohmann::json CellInfo::toJsonCell() const  {
    nlohmann::json val = nlohmann::json::object();
	val["ecgi"] = ecgi_.toJson();

	int value;
	value = collector_->get_dl_gbr_prb_usage_cell();
	if(value != -1) val["dl_gbr_prb_usage_cell"] = value;
	
	value = collector_->get_ul_gbr_prb_usage_cell();
	if(value != -1) val["ul_gbr_prb_usage_cell"] = value;
	 
	value = collector_->get_dl_nongbr_prb_usage_cell();
	if(value != -1) val["dl_nongbr_prb_usage_cell"] = value;

	value = collector_->get_ul_nongbr_prb_usage_cell();
	if(value != -1) val["ul_nongbr_prb_usage_cell"] = value;

	value = collector_->get_dl_total_prb_usage_cell();
//	if(value != -1)
	    val["dl_total_prb_usage_cell"] = value;

	value = collector_->get_ul_total_prb_usage_cell();
//	if(value != -1)
	    val["ul_total_prb_usage_cell"] = value;

	value = collector_->get_received_dedicated_preambles_cell();
	if(value != -1) val["received_dedicated_preambles_cell"] = value;

	value = collector_->get_received_randomly_selected_preambles_low_range_cell();
	if(value != -1) val["received_randomly_selected_preambles_low_range_cell"] = value;

	value = collector_->get_received_randomly_selected_preambles_high_range_cell();
	if(value != -1) val["received_randomly_selected_preambles_high_range_cell"] = value;
	
	value = collector_->get_number_of_active_ue_dl_gbr_cell();
	if(value != -1) val["number_of_active_ue_dl_gbr_cell"] = value;
	
	value = collector_->get_number_of_active_ue_ul_gbr_cell();
	if(value != -1) val["number_of_active_ue_ul_gbr_cell"] = value;
	
	value = collector_->get_number_of_active_ue_dl_nongbr_cell();
	if(value != -1) val["number_of_active_ue_dl_nongbr_cell"] = value;
	
	value = collector_->get_number_of_active_ue_ul_nongbr_cell();
	if(value != -1) val["number_of_active_ue_ul_nongbr_cell"] = value;
	
	value = collector_->get_dl_gbr_pdr_cell();
	if(value != -1) val["dl_gbr_pdr_cell"] = value;
	
	value = collector_->get_ul_gbr_pdr_cell();
	if(value != -1) val["ul_gbr_pdr_cell"] = value;
	
	value = collector_->get_dl_nongbr_pdr_cell();
	if(value != -1) val["dl_nongbr_pdr_cell"] = value;
	
	value = collector_->get_ul_nongbr_pdr_cell();
	if(value != -1) val["ul_nongbr_pdr_cell"] = value;

	return val;
}


nlohmann::json CellInfo::toJson() const {
	nlohmann::json val = toJsonCell();
	
//	// per UE
//	std::map<ipv4, ueCollector>::const_iterator it = ueList_->getBeginIterator();
//	std::map<ipv4, ueCollector>::const_iterator endIt = ueList_->getBeginIterator();
//
//    nlohmann::json jsonArray;
//
//	for(; it != endIt; ++it){
//		CellUeInfo ue(&(it->second));
//		jsonArray.push_back(ue.toJson());
//	}
//
//	if(jsonArray.size() > 0)
//	{
//		val["cellUeInfo"] = jsonArray;
//	}
	return val;
}


//nlohmann::json CellInfo::toJson(std::vector<Ipv4>& uesID) const {
//	nlohmann::json val = toJsonCell();
//	nlohmann::json jsonArray;
//	std::vector<Ipv4>&::const_iterator it = uesID.begin();
//	for(; it != uesID.end() ; ++it){
//		if(ueList_.findUeByIpv4(*it) != false){
//			CellUeInfo ue(ueList_.getUserCollectorByIPv4(*it));
//			jsonArray.push_back(ue.toJson());
//		}
//
//	}
//	if(jsonArray.size() > 0)
//	{
//		val["cellUeInfo"] = jsonArray;
//	}
//
//	return val;
//}
