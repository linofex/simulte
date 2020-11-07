//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "PacketFlowManager.h"
#include "corenetwork/statsCollector/EnodeBStatsCollector.h"
#include "corenetwork/statsCollector/UeStatsCollector.h"



Define_Module(PacketFlowManager);

PacketFlowManager::PacketFlowManager()
{
    eNBcollector_ = nullptr;
    ueCollector_  = nullptr;
}

void PacketFlowManager::initialize(int stage)
{
    if (stage == 1)
    {
        connectionMap_.clear();
        //                                                         LTE Nic          Enodeb
        eNBcollector_ = check_and_cast<EnodeBStatsCollector *>(getParentModule()->getParentModule()->getSubmodule("collector"));
    }
}

bool PacketFlowManager::checkLcid(LogicalCid lcid)
{
    if (connectionMap_.find(lcid) == connectionMap_.end())
        return false;
    return true;
}

void PacketFlowManager::initLcid(LogicalCid lcid, MacNodeId nodeId)
{
    if (connectionMap_.find(lcid) != connectionMap_.end())
        throw cRuntimeError("PacketFlowManager::initLcid - Logical CID %d already present. Aborting", lcid);

    // init new descriptor
    StatusDescriptor newDesc;
    newDesc.rlcPdusPerSdu_.clear();
    newDesc.rlcPdusPerSduDiscard_.clear();
    newDesc.rlcSdusPerPdu_.clear();
    newDesc.macSdusPerPdu_.clear();
    newDesc.macPduPerProcess_.resize(UE_TX_HARQ_PROCESSES, 0);

    connectionMap_[lcid] = newDesc;
    lcidToNodeIdmap_[lcid] = nodeId;
    EV << NOW << " PacketFlowManager::initLcid - initialized lcid " << lcid << endl;
}

void PacketFlowManager::clearLcid(LogicalCid lcid)
{
    if (connectionMap_.find(lcid) == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        EV << NOW << " PacketFlowManager::clearLcid - Logical CID " << lcid << " not present." << endl;
        return;
    }
    else
    {
        connectionMap_[lcid].rlcPdusPerSdu_.clear();
        connectionMap_[lcid].rlcPdusPerSduDiscard_.clear();
        connectionMap_[lcid].rlcSdusPerPdu_.clear();
        connectionMap_[lcid].macSdusPerPdu_.clear();
        for (int i=0; i<UE_TX_HARQ_PROCESSES; i++)
            connectionMap_[lcid].macPduPerProcess_[i] = 0;
    }

    EV << NOW << " PacketFlowManager::clearLcid - cleared data structures for lcid " << lcid << endl;
}

void PacketFlowManager::clearAllLcid()
{
    connectionMap_.clear();
    EV << NOW << " D2DLocalSwitchManager::clearAllLcid - cleared data structures for all lcids "<< endl;
}

void PacketFlowManager::insertRlcPdu(LogicalCid lcid, unsigned int rlcSno, SequenceNumberSet& pdcpSnoSet)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        EV << NOW << " D2DLocalSwitchManager::insertRlcPdu - Logical CID " << lcid << " not present." << endl;
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    if (desc->rlcSdusPerPdu_.find(rlcSno) != desc->rlcSdusPerPdu_.end())
        throw cRuntimeError("D2DLocalSwitchManager::insertRlcPdu - RLC PDU SN %d already present for logical CID %d. Aborting", rlcSno, lcid);

    SequenceNumberSet::iterator sit = pdcpSnoSet.begin();
    for (; sit != pdcpSnoSet.end(); ++sit)
    {
        unsigned int pdcpSno = *sit;

        // store the RLC SDUs (PDCP PDUs) included in the RLC PDU
        desc->rlcSdusPerPdu_[rlcSno].insert(pdcpSno);

        // now store the inverse association, i.e., for each RLC SDU, record in which RLC PDU is included
        desc->rlcPdusPerSdu_[pdcpSno].insert(rlcSno);
        desc->rlcPdusPerSduDiscard_[pdcpSno].insert(rlcSno);


        EV << NOW << " D2DLocalSwitchManager::insertRlcPdu - lcid[" << lcid << "], insert PDCP PDU " << pdcpSno << " in RLC PDU " << rlcSno << endl;
    }
}



void PacketFlowManager::discardRlcPdu(LogicalCid lcid, unsigned int rlcSno)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        EV << NOW << " D2DLocalSwitchManager::discardRlcPdu - Logical CID " << lcid << " not present." << endl;
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    if (desc->rlcSdusPerPdu_.find(rlcSno) != desc->rlcSdusPerPdu_.end())
        throw cRuntimeError("D2DLocalSwitchManager::discardRlcPdu - RLC PDU SN %d already present for logical CID %d. Aborting", rlcSno, lcid);

    // get the PCDP SDUs fragmented in this RLC PDU
    SequenceNumberSet pdcpSnoSet = desc->rlcSdusPerPdu_.find(rlcSno)->second;
    SequenceNumberSet::iterator sit = pdcpSnoSet.begin();
    for (; sit != pdcpSnoSet.end(); ++sit)
    {
        unsigned int pdcpSno = *sit;

        // remove the RLC PDUs that contains a fragment of this pdcpSno
        desc->rlcPdusPerSduDiscard_[pdcpSno].erase(rlcSno);
        if(desc->rlcPdusPerSduDiscard_[pdcpSno].empty())
        {
            EV << NOW << " D2DLocalSwitchManager::discardRlcPdu - lcid[" << lcid << "], discarded PDCP PDU " << pdcpSno << " in RLC PDU " << rlcSno << endl;
            pktDiscardCounterPerUe_[lcidToNodeIdmap_[lcid]] += 1;
            pktDiscardCounterTotal_ += 1;
        }
    }
    desc->rlcSdusPerPdu_.erase(rlcSno);
}

void PacketFlowManager::notifyStats(){
//    EnodeBStatsCollector->metodocheinserisceilvalore
    //per ogni utente chiama notifyue
}


void PacketFlowManager::notifyStatsToUe(MacNodeId nodeId){
    // guardo se ue è in questa cella (magari handover)
    // se lo è ritorno il puntatore e chiamo relativa funzione
    // se non lo è non lo invio (handover??)
}

void PacketFlowManager::resetCounters()
{
    pktDiscardCounterTotal_ = 0;

    std::map<MacNodeId, int>::iterator it  = pktDiscardCounterPerUe_.begin();
    std::map<MacNodeId, int>::iterator end = pktDiscardCounterPerUe_.end();
    for(; it != end; ++it)
    {
        it->second = 0;
    }
}

int PacketFlowManager::getDiscartedPkt(MacNodeId nodeId){
    std::map<MacNodeId, int>::iterator it = pktDiscardCounterPerUe_.find(nodeId);
    if (it == pktDiscardCounterPerUe_.end())
    {
        throw cRuntimeError("D2DLocalSwitchManager::getDiscartedPkt - nodeId [%d] not present", nodeId);
    }
    return it->second;
}


