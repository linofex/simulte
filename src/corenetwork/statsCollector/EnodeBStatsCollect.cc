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
#include "stack/packetFlowManager/PacketFlowManagerEnb.h"

#include "stack/pdcp_rrc/layer/LtePdcpRrc.h"
#include "stack/mac/layer/LteMacEnb.h"
#include <string>

Define_Module(EnodeBStatsCollector);

EnodeBStatsCollector::~EnodeBStatsCollector()
{
    cancelAndDelete(pdcpBytes_);
    cancelAndDelete(prbUsage_);
    cancelAndDelete(discardRate_);
    cancelAndDelete(activeUsers_);
    cancelAndDelete(packetDelay_);

}

void EnodeBStatsCollector::initialize(int stage){
    if (stage == 4)//inet::INITSTAGE_LOCAL)
    {

        ecgi_.plmn.mcc = getAncestorPar("mcc").stdstringValue();
        ecgi_.plmn.mnc = getAncestorPar("mnc").stdstringValue();

        mac_ = check_and_cast<LteMacEnb *>(getParentModule()->getSubmodule("lteNic")->getSubmodule("mac"));
        pdcp_ = check_and_cast<LtePdcpRrcEnb *>(getParentModule()->getSubmodule("lteNic")->getSubmodule("pdcpRrc"));
        flowManager_ = check_and_cast<PacketFlowManagerEnb *>(getParentModule()->getSubmodule("lteNic")->getSubmodule("packetFlowManager"));

        cellInfo_ = check_and_cast<LteCellInfo *>(getParentModule()->getSubmodule("cellInfo"));
        ecgi_.cellId = std::to_string(cellInfo_->getMacCellId());
        dl_total_prb_usage_cell.init("dl_total_prb_usage_cell", par("prbUsagePeriods"), par("movingAverage"));
        ul_total_prb_usage_cell.init("ul_total_prb_usage_cell", par("prbUsagePeriods"), par("movingAverage"));
        number_of_active_ue_dl_nongbr_cell.init("number_of_active_ue_dl_nongbr_cell", par("activeUserPeriods"), false);
        number_of_active_ue_ul_nongbr_cell.init("number_of_active_ue_ul_nongbr_cell", par("activeUserPeriods"), false);
        dl_nongbr_pdr_cell.init("dl_nongbr_pdr_cell", par("discardRatePeriods"), false);
        ul_nongbr_pdr_cell.init("ul_nongbr_pdr_cell", par("discardRatePeriods"), false);

        // setup timer
        prbUsage_ = new cMessage("prbUsage_");
        activeUsers_ = new cMessage("activeUsers_");
        discardRate_ = new cMessage("discardRate_");
        packetDelay_ = new cMessage("packetDelay_");
        pdcpBytes_ = new cMessage("pdcpBytes_");

        prbUsagePeriod_    = par("prbUsagePeriod");
        activeUsersPeriod_ = par("activeUserPeriod");
        discardRatePeriod_ = par("discardRatePeriod");
        delayPacketPeriod_ = par("delayPacketPeriod");
        dataVolumePeriod_   = par("dataVolumePeriod");

        // start scheduling the l2 meas
        scheduleAt(NOW + prbUsagePeriod_, prbUsage_);
        scheduleAt(NOW + activeUsersPeriod_, activeUsers_);
        scheduleAt(NOW + discardRatePeriod_, discardRate_);
        scheduleAt(NOW + delayPacketPeriod_, packetDelay_);
        scheduleAt(NOW + dataVolumePeriod_, pdcpBytes_);
    }
}


void EnodeBStatsCollector::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
    {
        if(strcmp(msg->getName(),"prbUsage_") == 0)
        {
            add_dl_total_prb_usage_cell();
            add_ul_total_prb_usage_cell();
            scheduleAt(NOW + prbUsagePeriod_, prbUsage_);
        }
        else if(strcmp(msg->getName(), "activeUsers_") == 0)
        {
            add_number_of_active_ue_dl_nongbr_cell();
            add_number_of_active_ue_ul_nongbr_cell();
            scheduleAt(NOW + activeUsersPeriod_, activeUsers_);

        }
        else if(strcmp(msg->getName(), "discardRate_") == 0)
        {
            add_dl_nongbr_pdr_cell();

            // add packet discard rate stats for each user
            add_dl_nongbr_pdr_cell_perUser();

            //reset counters
            pdcp_->resetPktCounter();
            flowManager_->resetDiscardCounter();
            resetDiscardCounterPerUe();
            scheduleAt(NOW + discardRatePeriod_, discardRate_);

        }
        else if(strcmp(msg->getName(), "packetDelay_") == 0)
        {
            add_dl_nongbr_delay_perUser();
            //reset counter
            resetDelayCounterPerUe();
            scheduleAt(NOW + delayPacketPeriod_, packetDelay_);
        }
        else if(strcmp(msg->getName(), "pdcpBytes_") == 0)
        {
            add_ul_nongbr_data_volume_ue_perUser();
            add_dl_nongbr_data_volume_ue_perUser();
            resetBytesCountersPerUe();
            scheduleAt(NOW + dataVolumePeriod_, pdcpBytes_);
        }
        else
        {
          delete msg;
        }

    }

}


void EnodeBStatsCollector::resetDiscardCounterPerUe()
{
    UeStatsCollectorMap::iterator it = ueCollectors_.begin();
    UeStatsCollectorMap::iterator end = ueCollectors_.end();
    for(; it != end ; ++it)
    {
        pdcp_->resetPktCounterPerUe(it->first);
        flowManager_->resetDiscardCounterPerUe(it->first);
    }
}

void EnodeBStatsCollector::resetDelayCounterPerUe()
{
    UeStatsCollectorMap::iterator it = ueCollectors_.begin();
    UeStatsCollectorMap::iterator end = ueCollectors_.end();
    for(; it != end ; ++it)
    {
        flowManager_->resetDelayCounterPerUe(it->first);
    }
}


void EnodeBStatsCollector::resetThroughputCountersPerUe()
{
    UeStatsCollectorMap::iterator it = ueCollectors_.begin();
        UeStatsCollectorMap::iterator end = ueCollectors_.end();
        for(; it != end ; ++it)
        {
            flowManager_->resetThroughputCounterPerUe(it->first);
        }
}

void EnodeBStatsCollector::resetBytesCountersPerUe()
{
    UeStatsCollectorMap::iterator it = ueCollectors_.begin();
    UeStatsCollectorMap::iterator end = ueCollectors_.end();
    for(; it != end ; ++it)
    {
        pdcp_->resetPdcpBytesDlPerUe(it->first);
        pdcp_->resetPdcpBytesUlPerUe(it->first);
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

UeStatsCollectorMap* EnodeBStatsCollector::getCollectorMap()
{
    return &ueCollectors_;
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
    double discard = flowManager_->getDiscardedPkt();
    dl_nongbr_pdr_cell.addValue(discard);
}

void EnodeBStatsCollector::add_ul_nongbr_pdr_cell()
{
    double pdr = 0;
    DiscardedPkts pair = {0,0};
    DiscardedPkts temp = {0,0};

    UeStatsCollectorMap::iterator it = ueCollectors_.begin();
    UeStatsCollectorMap::iterator end = ueCollectors_.end();

    for(; it != end ; ++it)
    {
        temp = it->second->getULDiscardedPkt();
        pair.discarded += temp.discarded;
        pair.total += temp.total;
    }

    pdr = pair.discarded * 1000000/ pair.total;
    ul_nongbr_pdr_cell.addValue(pdr);
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
        discard = flowManager_->getDiscardedPktPerUe(it->first);
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

void EnodeBStatsCollector::add_dl_nongbr_throughput_ue_perUser()
{
    UeStatsCollectorMap::iterator it = ueCollectors_.begin();
    UeStatsCollectorMap::iterator end = ueCollectors_.end();
    double throughput;
    for(; it != end ; ++it)
    {
        throughput = flowManager_->getThroughputStatsPerUe(it->first);
        it->second->add_dl_nongbr_throughput_ue(throughput);
    }
}


int EnodeBStatsCollector::get_ul_nongbr_pdr_cell(){return 0;}


int EnodeBStatsCollector::get_dl_nongbr_pdr_cell()
{
    return dl_nongbr_pdr_cell.getMean();
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

const mec::Ecgi& EnodeBStatsCollector::getEcgi() const
{
    return ecgi_;
}

MacCellId EnodeBStatsCollector::getCellId()const
{
    return cellInfo_->getMacCellId();
}
