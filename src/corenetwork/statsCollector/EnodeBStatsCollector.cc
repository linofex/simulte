//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "corenetwork/statsCollector/EnodeBStatsCollector.h"
#include "corenetwork/statsCollector/UeStatsCollector.h"
#include "stack/packetFlowManager/PacketFlowManager.h"

#include "stack/pdcp_rrc/layer/LtePdcpRrc.h"
#include "stack/mac/layer/LteMacEnb.h"


Define_Module(EnodeBStatsCollector);


void EnodeBStatsCollector::initialize(int stage){
    if (stage == inet::INITSTAGE_LOCAL)
    {
        mac_ = check_and_cast<LteMacEnb *>(getParentModule()->getSubmodule("lteNic")->getSubmodule("mac"));
        pdcp_ = check_and_cast<LtePdcpRrcEnb *>(getParentModule()->getSubmodule("lteNic")->getSubmodule("pdcpRrc"));
        flowManager_ = check_and_cast<PacketFlowManager *>(getParentModule()->getSubmodule("lteNic")->getSubmodule("packetFlowManager"));

        dl_total_prb_usage_cell.init("dl_total_prb_usage_cell", par("ttiPeriodPRBUsage"), par("movingAverage"));
        ul_total_prb_usage_cell.init("ul_total_prb_usage_cell", par("ttiPeriodPRBUsage"), par("movingAverage"));
        number_of_active_ue_dl_nongbr_cell.init("number_of_active_ue_dl_nongbr_cell", par("numberOfSamples"), false);
        number_of_active_ue_ul_nongbr_cell.init("number_of_active_ue_ul_nongbr_cell", par("numberOfSamples"), false);
    }
}

// UeStatsCollector management methods

void EnodeBStatsCollector::addUeCollector(MacNodeId id, UeStatsCollector* ueCollector)
{
    if(ueCollectors_.find(id) == ueCollectors_.end())
    {
        ueCollectors_.insert(std::pair<MacNodeId,UeStatsCollector *>(id, ueCollector));
    }
    else
    {
        throw cRuntimeError("EnodeBStatsCollector::addUeCollector - UeStatsCollector already present for UE nodeid[%d]", id);
    }
}


void EnodeBStatsCollector::removeUeCollector(MacNodeId id)
{
    std::map<MacNodeId, UeStatsCollector*>::iterator it = ueCollectors_.find(id);
    if(it != ueCollectors_.end())
        {
            ueCollectors_.erase(it);
        }
        else
        {
            throw cRuntimeError("EnodeBStatsCollector::removeUeCollector - UeStatsCollector not present for UE nodeid[%d]", id);
        }
}

UeStatsCollector* EnodeBStatsCollector::getUeCollector(MacNodeId id)
{
    std::map<MacNodeId, UeStatsCollector*>::iterator it = ueCollectors_.find(id);
    if(it != ueCollectors_.end())
    {
       return it->second;
    }
    else
    {
       throw cRuntimeError("EnodeBStatsCollector::removeUeCollector - UeStatsCollector not present for UE nodeid[%d]", id);
    }
}


bool EnodeBStatsCollector::hasUeCollector(MacNodeId id)
{
    return (ueCollectors_.find(id) != ueCollectors_.end()) ? true : false;
}



void EnodeBStatsCollector::add_dl_total_prb_usage_cell()
{
    double prb_usage = mac_->getUtilization(DL);
    dl_total_prb_usage_cell.addValue(prb_usage);
}
void EnodeBStatsCollector::add_ul_total_prb_usage_cell()
{
    double prb_usage = mac_->getUtilization(UL);
    ul_total_prb_usage_cell.addValue(prb_usage);
}
void EnodeBStatsCollector::add_number_of_active_ue_dl_nongbr_cell()
{
    int users = mac_->getActiveUeSetSize(DL);
    number_of_active_ue_dl_nongbr_cell.addValue(users);
}
void EnodeBStatsCollector::add_number_of_active_ue_ul_nongbr_cell()
{
    int users = mac_->getActiveUeSetSize(UL);
    number_of_active_ue_ul_nongbr_cell.addValue(users);
}
void EnodeBStatsCollector::add_dl_nongbr_pdr_cell()
{
    double discard = pdcp_->getDiscardRateStats();
    dl_nongbr_pdr_cell.addValue(discard);
}

// for each user save stats
//TODO handover management

void EnodeBStatsCollector::add_dl_nongbr_pdr_cell_perUser()
{
    UeStatsCollectorMap::iterator it = ueCollectors_.begin();
    UeStatsCollectorMap::iterator end = ueCollectors_.end();
    double discard;
    for(; it != end ; ++it)
    {
        discard = pdcp_->getDiscardRateStatsPerUe(it->first);
        it->second->add_dl_nongbr_pdr_ue(discard);
    }
}

void EnodeBStatsCollector::add_dl_nongbr_delay_perUser()
{
    UeStatsCollectorMap::iterator it = ueCollectors_.begin();
    UeStatsCollectorMap::iterator end = ueCollectors_.end();
    double delay;
    for(; it != end ; ++it)
    {
        delay = flowManager_->getDelayStatsPerUe(it->first);
        it->second->add_dl_nongbr_delay_ue(delay);
    }
}

void EnodeBStatsCollector::add_ul_nongbr_data_volume_ue_perUser()
{
    UeStatsCollectorMap::iterator it = ueCollectors_.begin();
    UeStatsCollectorMap::iterator end = ueCollectors_.end();
    unsigned int bytes;
    for(; it != end ; ++it)
    {
        bytes = pdcp_->getPdcpBytesUlPerUe(it->first);
        it->second->add_ul_nongbr_data_volume_ue(bytes);
    }
}

void EnodeBStatsCollector::add_dl_nongbr_data_volume_ue_perUser()
{
    UeStatsCollectorMap::iterator it = ueCollectors_.begin();
    UeStatsCollectorMap::iterator end = ueCollectors_.end();
    unsigned int bytes;
    for(; it != end ; ++it)
    {
        bytes = pdcp_->getPdcpBytesDlPerUe(it->first);
        it->second->add_dl_nongbr_data_volume_ue(bytes);
    }
}


void EnodeBStatsCollector::add_ul_nongbr_pdr_cell(){}


int EnodeBStatsCollector::get_ul_nongbr_pdr_cell(){}


int EnodeBStatsCollector::get_dl_nongbr_pdr_cell()
{
    double discard = pdcp_->getDiscardRateStats();
    dl_nongbr_pdr_cell.addValue(discard);
}

// since GBR rab has not been implemented nongbr = total
int EnodeBStatsCollector::get_dl_total_prb_usage_cell() {
    return dl_total_prb_usage_cell.getMean();
}

int EnodeBStatsCollector::get_ul_total_prb_usage_cell() {
    return ul_total_prb_usage_cell.getMean();
}

int EnodeBStatsCollector::get_dl_nongbr_prb_usage_cell() {
    return dl_total_prb_usage_cell.getMean();
}

int EnodeBStatsCollector::get_ul_nongbr_prb_usage_cell() {
    return ul_total_prb_usage_cell.getMean();
}


int EnodeBStatsCollector::get_number_of_active_ue_dl_nongbr_cell(){
    return number_of_active_ue_dl_nongbr_cell.getMean();
}

int EnodeBStatsCollector::get_number_of_active_ue_ul_nongbr_cell(){
    return number_of_active_ue_ul_nongbr_cell.getMean();
}


