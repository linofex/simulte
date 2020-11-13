//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "PacketFlowManager.h"
#include "stack/mac/layer/LteMacBase.h"

#include <sstream>
Define_Module(PacketFlowManager);

PacketFlowManager::PacketFlowManager()
{
    connectionMap_.clear();
    lcidToNodeIdmap_.clear();
    pktDiscardCounterPerUe_.clear();
    pdcpDelay_.clear();
//    times_.setName("delay times");
}

void PacketFlowManager::initialize(int stage)
{
    if (stage == 1)
    {
        LteMacBase * mac_= check_and_cast<LteMacBase *>(getParentModule()->getSubmodule("mac"));
        nodeType_ = mac_->getNodeType();
        harqProcesses_ = (nodeType_ == UE) ? UE_TX_HARQ_PROCESSES : ENB_TX_HARQ_PROCESSES;

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
    newDesc.nodeId_ = nodeId;
    newDesc.pdcpSduEntryTime_.clear();
    newDesc.rlcPdusPerSdu_.clear();
    newDesc.rlcPdusPerSduDiscard_.clear();
    newDesc.rlcSdusPerPdu_.clear();
    newDesc.macSdusPerPdu_.clear();
    newDesc.macPduPerProcess_.resize(harqProcesses_, 0);


    std::stringstream strs;
      strs << nodeId -1025;
      std::string temp_str = strs.str();
      char* char_type = (char*) temp_str.c_str();
    times_[nodeId].setName(char_type);



    connectionMap_[lcid] = newDesc;
    lcidToNodeIdmap_[lcid] = nodeId;
    pktDiscardCounterPerUe_[nodeId] = 0;
    EV_INFO << NOW << " PacketFlowManager::initLcid - initialized lcid " << lcid << endl;
}

void PacketFlowManager::clearLcid(LogicalCid lcid)
{
    if (connectionMap_.find(lcid) == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        EV_INFO << NOW << " PacketFlowManager::clearLcid - Logical CID " << lcid << " not present." << endl;
        return;
    }
    else
    {
        connectionMap_[lcid].pdcpSduEntryTime_.clear();
        connectionMap_[lcid].rlcPdusPerSdu_.clear();
        connectionMap_[lcid].rlcPdusPerSduDiscard_.clear();
        connectionMap_[lcid].rlcSdusPerPdu_.clear();
        connectionMap_[lcid].macSdusPerPdu_.clear();
        for (int i=0; i<harqProcesses_; i++)
            connectionMap_[lcid].macPduPerProcess_[i] = 0;
        // does miss connectionMap_.erase(lcid)?
    }

    EV_INFO << NOW << " PacketFlowManager::clearLcid - cleared data structures for lcid " << lcid << endl;
}

void PacketFlowManager::clearAllLcid()
{
    connectionMap_.clear();
    EV_INFO << NOW << " PacketFlowManager::clearAllLcid - cleared data structures for all lcids "<< endl;
}

void PacketFlowManager::insertPdcpSdu(LogicalCid lcid, unsigned int pdcpSno, simtime_t arrivalTime)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_INFO << NOW << " PacketFlowManager::insertRlcPdu - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManager::insertPdcpSdu - Logical CID %d not present. It must be initialized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    if (desc->pdcpSduEntryTime_.find(pdcpSno) != desc->pdcpSduEntryTime_.end())
        throw cRuntimeError("PacketFlowManager::insertPdcpSdu - PDCP PDU SN %d already present for logical CID %d. Aborting", pdcpSno, lcid);

    desc->pdcpSduEntryTime_[pdcpSno] = arrivalTime;

    EV_INFO << NOW << " PacketFlowManager::insertPdcpSdu - PDCP pdu "<< pdcpSno << " Added to Logical CID " << lcid << endl;

    // add user to delay time map if non already present since many lcids can belong to a one nodeId (UE)
    if(pdcpDelay_.find(desc->nodeId_) == pdcpDelay_.end())
        pdcpDelay_.insert(std::pair<unsigned int, std::pair<simtime_t, unsigned int> >(desc->nodeId_ ,std::make_pair(0,0)));
}

void PacketFlowManager::insertRlcPdu(LogicalCid lcid, unsigned int rlcSno, SequenceNumberSet& pdcpSnoSet,  bool lastIsFrag)
{
    EV_INFO << NOW << " PacketFlowManager::insertRlcPdu - Logical CID " << lcid << endl;
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_INFO << NOW << " PacketFlowManager::insertRlcPdu - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManager::insertRlcPdu - Logical CID %d not present. It must be initialized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    if (desc->rlcSdusPerPdu_.find(rlcSno) != desc->rlcSdusPerPdu_.end())
        throw cRuntimeError("PacketFlowManager::insertRlcPdu - RLC PDU SN %d already present for logical CID %d. Aborting", rlcSno, lcid);

    SequenceNumberSet::iterator sit = pdcpSnoSet.begin();
    for (; sit != pdcpSnoSet.end(); ++sit)
    {
        unsigned int pdcpSno = *sit;

        // store the RLC SDUs (PDCP PDUs) included in the RLC PDU
        desc->rlcSdusPerPdu_[rlcSno].insert(pdcpSno);

        // now store the inverse association, i.e., for each RLC SDU, record in which RLC PDU is included
        desc->rlcPdusPerSdu_[pdcpSno].insert(rlcSno);
        desc->rlcPdusPerSduDiscard_[pdcpSno].insert(rlcSno);

        if(sit != pdcpSnoSet.end() && sit == --pdcpSnoSet.end()){
            desc->allPdcdSduToRlc_[pdcpSno] = !lastIsFrag;
        }else{
            desc->allPdcdSduToRlc_[pdcpSno] = true;
        }

        // take into account that a pdpc pdu has been fragmented in successive rlc pdus
//        if(sit != pdcpSnoSet.end() && sit == --pdcpSnoSet.end()){
//            if(lastIsFrag)
//            {
//                EV_INFO << NOW << " PacketFlowManager::insertRlcPdu - Logical CID " << lcid << " has pdcp pdu " << pdcpSno  <<" curr " << lastIsFrag<< endl;
//                desc->allPdcdSduToRlc_[pdcpSno] = false;
//            }
//        }
//        else{
//            // if a fragmented pdpc pdu is not the last pdcp in the rlc means that the encapsulation into a
//            // rlc pdu is completed
//            if(desc->allPdcdSduToRlc_.find(pdcpSno) != desc->allPdcdSduToRlc_.end())
//                desc->allPdcdSduToRlc_[pdcpSno] = true;
//        }

        EV_INFO << NOW << " PacketFlowManager::insertRlcPdu - lcid[" << lcid << "], insert PDCP PDU " << pdcpSno << " in RLC PDU " << rlcSno << endl;
    }
}

void PacketFlowManager::discardRlcPdu(LogicalCid lcid, unsigned int rlcSno)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_INFO << NOW << " PacketFlowManager::discardRlcPdu - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManager::discardRlcPdu - Logical CID %d not present. It must be initilized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    if (desc->rlcSdusPerPdu_.find(rlcSno) != desc->rlcSdusPerPdu_.end())
        throw cRuntimeError("PacketFlowManager::discardRlcPdu - RLC PDU SN %d already present for logical CID %d. Aborting", rlcSno, lcid);

    // get the PCDP SDUs fragmented in this RLC PDU
    SequenceNumberSet pdcpSnoSet = desc->rlcSdusPerPdu_.find(rlcSno)->second;
    SequenceNumberSet::iterator sit = pdcpSnoSet.begin();
    for (; sit != pdcpSnoSet.end(); ++sit)
    {
        unsigned int pdcpSno = *sit;

        // remove the RLC PDUs that contains a fragment of this pdcpSno
        desc->rlcPdusPerSduDiscard_[pdcpSno].erase(rlcSno);

        // if the set is empy and the pdcp pdu has been encapsulated all
        if(desc->rlcPdusPerSduDiscard_[pdcpSno].empty() && desc->allPdcdSduToRlc_[pdcpSno])
        {
            EV_INFO << NOW << " PacketFlowManager::discardRlcPdu - lcid[" << lcid << "], discarded PDCP PDU " << pdcpSno << " in RLC PDU " << rlcSno << endl;
            pktDiscardCounterPerUe_[lcidToNodeIdmap_[lcid]] += 1;
            pktDiscardCounterTotal_ += 1;
        }
    }
    //remove discarded rlc pdu
    desc->rlcSdusPerPdu_.erase(rlcSno);
}

void PacketFlowManager::insertMacPdu(LogicalCid lcid, unsigned int macPduId, SequenceNumberSet& rlcSnoSet)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_INFO << NOW << " PacketFlowManager::insertMacPdu - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManager::insertMacPdu - Logical CID %d not present. It must be initilized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    if (desc->macSdusPerPdu_.find(macPduId) != desc->macSdusPerPdu_.end())
        throw cRuntimeError("PacketFlowManager::insertMacPdu - MAC PDU ID %d already present for logical CID %d. Aborting", macPduId, lcid);

    SequenceNumberSet::iterator sit = rlcSnoSet.begin();
    for (; sit != rlcSnoSet.end(); ++sit)
    {
        unsigned int rlcSno = *sit;

        // ??????
        // record the associaton RLC PDU - MAC PDU only if the RLC PDU contains at least one entire SDU
        // the condition holds if the RLC PDU SN is stored in the data structure rlcSdusPerPdu_
        if (desc->rlcSdusPerPdu_.find(rlcSno) != desc->rlcSdusPerPdu_.end())
        {
            // store the MAC SDUs (RLC PDUs) included in the MAC PDU
            desc->macSdusPerPdu_[macPduId].insert(rlcSno);
            EV_INFO << NOW << " PacketFlowManager::insertMacPdu - lcid[" << lcid << "], insert RLC PDU " << rlcSno << " in MAC PDU " << macPduId << endl;
        }
    }
}

void PacketFlowManager::macPduArrived(LogicalCid lcid, unsigned int macPdu)
{

EV_INFO << NOW << " PacketFlowManager::macPduArrived - MAC PDU "<< macPdu << " of lcid " << lcid << " arrived." << endl;

    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_INFO << NOW << " PacketFlowManager::notifyHarqProcess - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManager::insertMacPdu - Logical CID %d not present. It must be initilized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;

    EV_INFO << NOW << " PacketFlowManager::macPduArrived - Get MAC PDU ID [" << macPdu << "], which contains:" << endl;

    // === STEP 1 ==================================================== //
    // === recover the set of RLC PDU SN from the above MAC PDU ID === //

    std::map<unsigned int, SequenceNumberSet>::iterator mit = desc->macSdusPerPdu_.find(macPdu);
    if (mit == desc->macSdusPerPdu_.end())
        throw cRuntimeError("PacketFlowManager::macPduArrived - MAC PDU ID %d not present for logical CID %d. Aborting", macPdu, lcid);
    SequenceNumberSet rlcSnoSet = mit->second;

    // === STEP 2 ========================================================== //
    // === for each RLC PDU SN, recover the set of RLC SDU (PDCP PDU) SN === //

    SequenceNumberSet::iterator it = rlcSnoSet.begin();
    for (; it != rlcSnoSet.end(); ++it)
    {
        // for each RLC PDU
        unsigned int rlcPduSno = *it;

        EV_INFO << NOW << " PacketFlowManager::macPduArrived - --> RLC PDU [" << rlcPduSno << "], which contains:" << endl;

        std::map<unsigned int, SequenceNumberSet>::iterator nit = desc->rlcSdusPerPdu_.find(rlcPduSno);
        if (nit == desc->rlcSdusPerPdu_.end())
            throw cRuntimeError("PacketFlowManager::macPduArrived - RLC PDU SN %d not present for logical CID %d. Aborting", rlcPduSno, lcid);
        SequenceNumberSet pdcpSnoSet = nit->second;

        // === STEP 3 ============================================================================ //
        // === Since a RLC SDU may be fragmented in more than one RLC PDU, thus it must be     === //
        // === retransmitted only if all fragments have been transmitted.                      === //
        // === For each RLC SDU (PDCP PDU) SN, recover the set of RLC PDU where it is included,=== //
        // === remove the above RLC PDU SN. If the set becomes empty, compute the delay if     === //
        // === all PDCP PDU fragments have been transmitted                                     === //
        
        SequenceNumberSet::iterator jt = pdcpSnoSet.begin();
        for (; jt != pdcpSnoSet.end(); ++jt)
        {
            // for each RLC SDU (PDCP PDU), get the set of RLC PDUs where it is included
            unsigned int pdcpPduSno = *jt;

            EV_INFO << NOW << " PacketFlowManager::macPduArrived - ----> PDCP PDU [" << pdcpPduSno << "]" << endl;

            std::map<unsigned int, SequenceNumberSet>::iterator oit = desc->rlcPdusPerSdu_.find(pdcpPduSno);
            if (oit == desc->rlcPdusPerSdu_.end())
                throw cRuntimeError("PacketFlowManager::macPduArrived - PDCP PDU SN %d not present for logical CID %d. Aborting", pdcpPduSno, lcid);

            // oit->second is the set of RLC PDU in which the PDCP PDU is contained

            // the RLC PDU SN must be present in the set
            SequenceNumberSet::iterator kt = oit->second.find(rlcPduSno);
            if (kt == oit->second.end())
                 throw cRuntimeError("PacketFlowManager::macPduArrived - RLC PDU SN %d not present in the set of PDCP PDU SN %d for logical CID %d. Aborting", pdcpPduSno, rlcPduSno, lcid);

            // the RLC PDU has been sent, so erase it from the set
            oit->second.erase(kt);

            // check whether the set is now empty
            if (desc->rlcPdusPerSdu_[pdcpPduSno].empty())
            {
                // set the time for pdcpPduSno
                std::map<unsigned int, simtime_t>::iterator tit = desc->pdcpSduEntryTime_.find(pdcpPduSno);
                if(tit == desc->pdcpSduEntryTime_.end())
                    throw cRuntimeError("PacketFlowManager::macPduArrived - PDCP PDU SN %d of Lcid %d has not an entry time timestamp, this should not happen. Aborting", pdcpPduSno, lcid);

                std::map<unsigned int, bool>::iterator bit = desc->allPdcdSduToRlc_.find(pdcpPduSno);
                bool allPdcp = true;
                // if the entry is not present, the pdcp is directly all encapsulated into rlc
                // otherwise set the flag according to the pdcp state
                if(bit == desc->allPdcdSduToRlc_.end())
                    throw cRuntimeError("PacketFlowManager::macPduArrived");

                if(bit->second)
                { // the whole current pdcp seqNum has been received
                    EV_INFO << NOW << " PacketFlowManager::macPduArrived - ----> PDCP PDU [" << pdcpPduSno << "] has been completely sent, remove from PDCP buffer" << endl;

                    delayMap::iterator dit = pdcpDelay_.find(desc->nodeId_);
                    if(dit == pdcpDelay_.end())
                        throw cRuntimeError("PacketFlowManager::macPduArrived - Node id %d is not in pdcp delay map structure, this should not happen. Aborting", desc->nodeId_);

                    int time = (simTime() - tit->second).dbl() *1000;

                    EV_INFO << NOW << " PacketFlowManager::macPduArrived - PDCP PDU "<< pdcpPduSno << " of lcid " << lcid << " acknowledged. Delay time: " << time << "ms"<< endl;
                    times_[lcidToNodeIdmap_[lcid]].record(time);
                    dit->second.first += (simTime() - tit->second);
                    dit->second.second += 1;

                    //remove pdcp sdu from entry time
                    desc->pdcpSduEntryTime_.erase(tit);

                    // update next sno
                    nextPdcpSno_ = pdcpPduSno+1;

                    oit->second.clear();
                    desc->rlcPdusPerSdu_.erase(oit); // erase PDCP PDU SN
                    desc->rlcPdusPerSduDiscard_.erase(oit->first);
                }
            }
        }

        nit->second.clear();
        desc->rlcSdusPerPdu_.erase(nit); // erase RLC PDU SN

        // update next sno
        nextRlcSno_ = rlcPduSno+1;
    }

    mit->second.clear();
    desc->macSdusPerPdu_.erase(mit); // erase MAC PDU ID

}

void PacketFlowManager::resetCounterPerUe(MacNodeId id)
{
    std::map<MacNodeId, int>::iterator it = pktDiscardCounterPerUe_.find(id);
    if (it == pktDiscardCounterPerUe_.end())
    {
        // maybe it is possible? think about it
        throw cRuntimeError("PacketFlowManager::resetCounterPerUe - nodeId [%d] not present", id);
    }
    it->second = 0;
}

void PacketFlowManager::resetCounter()
{
    pktDiscardCounterTotal_ = 0;
}

int PacketFlowManager::getTotalDiscardedPckPerUe(MacNodeId id)
{
    std::map<MacNodeId, int>::iterator it = pktDiscardCounterPerUe_.find(id);
    if (it == pktDiscardCounterPerUe_.end())
    {
        // maybe it is possible? think about it
        throw cRuntimeError("PacketFlowManager::getTotalDiscardedPckPerUe - nodeId [%d] not present", id);
        return 0;
    }
    return it->second;
    }

int PacketFlowManager::getTotalDiscardedPck()
{
    return pktDiscardCounterTotal_;
}

double PacketFlowManager::getDelayStatsPerUe(MacNodeId id)
{
    delayMap::iterator it = pdcpDelay_.find(id);
    if (it == pdcpDelay_.end())
    {
        // this may occur after a handover, when data structures are cleared
        EV_INFO << NOW << " PacketFlowManager::getDelayStatsPerUe - Delay Stats for Node Id " << id << " not present." << endl;
        return 0;
    }

    double totalMs = (it->second.first.dbl())*1000;
    double delayMean = totalMs / it->second.second;
    return delayMean;
}

void PacketFlowManager::resetDelayCounterPerUe(MacNodeId id)
{
    delayMap::iterator it = pdcpDelay_.find(id);
    if (it == pdcpDelay_.end())
    {
       // this may occur after a handover, when data structures are cleared
       EV_INFO << NOW << " PacketFlowManager::getDelayStatsPerUe - Delay Stats for Node Id " << id << " not present." << endl;
       return;
    }

    it->second = std::make_pair<simtime_t, unsigned int>(0, 0.);

    //or
    // it->second.fisrt = 0
    // it->second.second = 0
}

void PacketFlowManager::deleteUe(MacNodeId id)
{
    /* It has to be deleted:
     * all structures with MacNodeId id
     * all lcid belongs to MacNodeId id
     */
    std::map<LogicalCid, StatusDescriptor>::iterator it = connectionMap_.begin();

    while(it != connectionMap_.end())
    {
        if(it->second.nodeId_ == id)
        {
            lcidToNodeIdmap_.erase(it->first);
            connectionMap_.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    std::map<MacNodeId, int>::iterator pit = pktDiscardCounterPerUe_.find(id);
    if( pit != pktDiscardCounterPerUe_.end())
        pktDiscardCounterPerUe_.erase(pit);

    delayMap::iterator dit = pdcpDelay_.find(id);
    if(dit != pdcpDelay_.end())
        pdcpDelay_.erase(dit);
}
