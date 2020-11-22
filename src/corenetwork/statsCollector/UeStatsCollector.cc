//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "corenetwork/statsCollector/UeStatsCollector.h"
#include "stack/pdcp_rrc/layer/LtePdcpRrc.h"
#include "stack/mac/layer/LteMacUe.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/common/ModuleAccess.h"
#include "stack/packetFlowManager/PacketFlowManagerUe.h"

Define_Module(UeStatsCollector);


void UeStatsCollector::initialize(int stage){
    if (stage == INITSTAGE_NETWORK_LAYER_3) // same as lteMacUe, when read the interface entry
    {
        associateId_.type = "1"; // UE_IPV4_ADDRESS
        // find interface entry and use its address
        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        // TODO: how do we find the LTE interface?
        InterfaceEntry * interfaceEntry = interfaceTable->getInterfaceByName("wlan");
//
        IPv4InterfaceData* ipv4if = interfaceEntry->ipv4Data();
        if(ipv4if == nullptr)
            throw cRuntimeError("UeStatsCollector::initialize - no IPv4 interface data");
        associateId_.value = ipv4if->getIPAddress().str();
        mac_ = check_and_cast<LteMacUe *>(getParentModule()->getSubmodule("lteNic")->getSubmodule("mac"));
        pdcp_ = check_and_cast<LtePdcpRrcUe *>(getParentModule()->getSubmodule("lteNic")->getSubmodule("pdcpRrc"));
        flowManager_ = check_and_cast<PacketFlowManagerUe *>(getParentModule()->getSubmodule("lteNic")->getSubmodule("packetFlowManager"));
        handover_ = false;

        // packet delay
         ul_nongbr_delay_ue.init("ul_nongbr_delay_ue", par("ttiPeriodPRBUsage"), par("movingAverage"));
         dl_nongbr_delay_ue.init("dl_nongbr_delay_ue",par("ttiPeriodPRBUsage"), par("movingAverage"));
        // packet discard rate
         ul_nongbr_pdr_ue.init("ul_nongbr_pdr_ue", par("ttiPeriodPRBUsage"), par("movingAverage"));
         dl_nongbr_pdr_ue.init("dl_nongbr_pdr_ue", par("ttiPeriodPRBUsage"), par("movingAverage"));
        // scheduled throughput
         ul_nongbr_throughput_ue.init("ul_nongbr_throughput_ue", par("ttiPeriodPRBUsage"), par("movingAverage"));
         dl_nongbr_throughput_ue.init("dl_nongbr_throughput_ue", par("ttiPeriodPRBUsage"), par("movingAverage"));
        // data volume
         ul_nongbr_data_volume_ue.init("ul_nongbr_data_volume_ue", par("ttiPeriodPRBUsage"), par("movingAverage"));
         dl_nongbr_data_volume_ue.init("dl_nongbr_data_volume_ue", par("ttiPeriodPRBUsage"), par("movingAverage"));
    }
}


void UeStatsCollector::add_ul_nongbr_delay_ue(){}

// called by the eNodeBCollector
void UeStatsCollector::add_dl_nongbr_delay_ue(double value)
{
    dl_nongbr_delay_ue.addValue(value);
}

void UeStatsCollector::add_ul_nongbr_pdr_ue(double value)
{
    ul_nongbr_pdr_ue.addValue(value);
}
// called by the eNodeBCollector
void UeStatsCollector::add_dl_nongbr_pdr_ue(double value)
{
    dl_nongbr_pdr_ue.addValue(value);
}

// called by the eNodeBCollector
void UeStatsCollector::add_ul_nongbr_throughput_ue(double value)
{
    ul_nongbr_throughput_ue.addValue(value);
}
void UeStatsCollector::add_dl_nongbr_throughput_ue(double value)
{
    dl_nongbr_throughput_ue.addValue(value);
}
// called by the eNodeBCollector
void UeStatsCollector::add_ul_nongbr_data_volume_ue(unsigned int value)
{
    ul_nongbr_data_volume_ue.addValue(value);
}
void UeStatsCollector::add_dl_nongbr_data_volume_ue(unsigned int value)
{
    dl_nongbr_data_volume_ue.addValue(value);
}


int UeStatsCollector::get_ul_nongbr_delay_ue()
{
    return ul_nongbr_delay_ue.getMean();
}
int UeStatsCollector::get_dl_nongbr_delay_ue()
{
    return dl_nongbr_delay_ue.getMean();
}

int UeStatsCollector::get_ul_nongbr_pdr_ue()
{
    return ul_nongbr_pdr_ue.getMean();
}
int UeStatsCollector::get_dl_nongbr_pdr_ue()
{
    return dl_nongbr_pdr_ue.getMean();
}

int UeStatsCollector::get_ul_nongbr_throughput_ue()
{
    return -1;}
int UeStatsCollector::get_dl_nongbr_throughput_ue()
{
    return -1;
}

int UeStatsCollector::get_ul_nongbr_data_volume_ue()
{
    return ul_nongbr_data_volume_ue.getMean();
}
int UeStatsCollector::get_dl_nongbr_data_volume_ue()
{
    return dl_nongbr_data_volume_ue.getMean();
}

DiscardedPkts UeStatsCollector::getULDiscardedPkt()
{
    DiscardedPkts pair;
    pair = flowManager_->getDiscardedPkt();
    double rate = pair.discarded * 1000000 / pair.total;
    add_ul_nongbr_pdr_ue(rate);
    return pair;
}
