//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "PacketFlowManagerEnb.h"
#include "stack/mac/layer/LteMacBase.h"
#include "stack/pdcp_rrc/layer/LtePdcpRrc.h"
#include "stack/rlc/packet/LteRlcPdu.h"
#include "stack/rlc/packet/LteRlcSdu.h"
#include "stack/mac/packet/LteMacPdu.h"
#include "stack/rlc/LteRlcDefs.h"
#include "stack/rlc/packet/LteRlcDataPdu.h"
#include "common/LteControlInfo.h"
#include <sstream>
Define_Module(PacketFlowManagerEnb);

PacketFlowManagerEnb::PacketFlowManagerEnb()
{
    connectionMap_.clear();
    pktDiscardCounterPerUe_.clear();
    pdcpDelay_.clear();
    pdcpThroughput_.clear();
    ee.setName("ee");
    ii.setName("ii");
    ww.setName("iii");

}

PacketFlowManagerEnb::~PacketFlowManagerEnb()
{
//times_.clear();
}

void PacketFlowManagerEnb::initialize(int stage)
{
    if (stage == 1)
    {
        PacketFlowManagerBase::initialize(stage);
        pdcp_ = check_and_cast<LtePdcpRrcEnb *>(getParentModule()->getSubmodule("pdcpRrc"));
        headerCompressedSize_ = pdcp_->par("headerCompressedSize");
    }
}

bool PacketFlowManagerEnb::checkLcid(LogicalCid lcid)
{
    if (connectionMap_.find(lcid) == connectionMap_.end())
        return false;
    return true;
}

void PacketFlowManagerEnb::initLcid(LogicalCid lcid, MacNodeId nodeId)
{
    if (connectionMap_.find(lcid) != connectionMap_.end())
        throw cRuntimeError("PacketFlowManagerEnb::initLcid - Logical CID %d already present. Aborting", lcid);

    // init new descriptor
    StatusDescriptor newDesc;
    newDesc.nodeId_ = nodeId;
    newDesc.burstId_ = 0;
    newDesc.burstState_ = false;
    newDesc.pdcpStatus_.clear();
    newDesc.rlcPdusPerSdu_.clear();
    newDesc.rlcSdusPerPdu_.clear();
    newDesc.macSdusPerPdu_.clear();
    newDesc.macPduPerProcess_.resize(harqProcesses_, 0);

    BurstStatus newBurstStatus;
    newBurstStatus.burstSize = 0;
    newBurstStatus.rlcPdu.clear();
    newBurstStatus.startBurstTransmission = -1;

    newDesc.burstStatus_.clear();

    std::stringstream strs;
      strs << nodeId -1025;
      std::string temp_str = strs.str();
      char* char_type = (char*) temp_str.c_str();
    times_[nodeId].setName(char_type);

    connectionMap_[lcid] = newDesc;
    pktDiscardCounterPerUe_[nodeId] = 0;
    EV_FATAL << NOW << "node id "<< nodeId << " PacketFlowManagerEnb::initLcid - initialized lcid " << lcid << endl;
}

void PacketFlowManagerEnb::clearLcid(LogicalCid lcid)
{
    if (connectionMap_.find(lcid) == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        EV_FATAL << NOW << " PacketFlowManagerEnb::clearLcid - Logical CID " << lcid << " not present." << endl;
        return;
    }
    else
    {
        connectionMap_[lcid].pdcpStatus_.clear();
        connectionMap_[lcid].rlcPdusPerSdu_.clear();
        connectionMap_[lcid].rlcSdusPerPdu_.clear();
        connectionMap_[lcid].macSdusPerPdu_.clear();
        connectionMap_[lcid].burstStatus_.clear();
        connectionMap_[lcid].burstId_ = 0;
        connectionMap_[lcid].burstState_ = false;


        for (int i=0; i<harqProcesses_; i++)
            connectionMap_[lcid].macPduPerProcess_[i] = 0;
    }

//    EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::clearLcid - cleared data structures for lcid " << lcid << endl;
}

void PacketFlowManagerEnb::clearAllLcid()
{
    connectionMap_.clear();
    EV_FATAL << NOW << " PacketFlowManagerEnb::clearAllLcid - cleared data structures for all lcids "<< endl;
}

void PacketFlowManagerEnb::initPdcpStatus(StatusDescriptor* desc, unsigned int pdcp, simtime_t& arrivalTime)
{
    // if pdcpStatus_ already present, error
    std::map<unsigned int, PdcpStatus>::iterator it = desc->pdcpStatus_.find(pdcp);
    if(it != desc->pdcpStatus_.end())
        throw cRuntimeError("PacketFlowManagerEnb::initPdcpStatus - PdcpStatus for PDCP sno [%d] already present, this should not happen. Abort", pdcp);

    PdcpStatus newpdcpStatus;

    newpdcpStatus.entryTime = arrivalTime;
    newpdcpStatus.discardedAtMac = false;
    newpdcpStatus.discardedAtRlc = false;
    newpdcpStatus.hasArrivedAll  = false;
    newpdcpStatus.sentOverTheAir =  false;
    newpdcpStatus.pdcpSduSize = 0;
    newpdcpStatus.entryTime = arrivalTime;

    desc->pdcpStatus_[pdcp] = newpdcpStatus;

}

void PacketFlowManagerEnb::insertPdcpSdu(LogicalCid lcid, unsigned int pdcpSno, simtime_t arrivalTime)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertRlcPdu - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManagerEnb::insertPdcpSdu - Logical CID %d not present. It must be initialized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;

    initPdcpStatus(desc, pdcpSno, arrivalTime);
    EV_FATAL << NOW << "node id "<< desc->nodeId_-1025 <<" PacketFlowManagerEnb::insertPdcpSdu - PDCP status for PDCP PDU SN " << pdcpSno<<" added. Logicl cid " << lcid << endl;


    // add user to delay time map if non already present since many lcids can belong to a one nodeId (UE)
    if(pdcpDelay_.find(desc->nodeId_) == pdcpDelay_.end())
        pdcpDelay_.insert(std::pair<unsigned int, delay>(desc->nodeId_ , {0,0}));

    if(pdcpThroughput_.find(desc->nodeId_) == pdcpThroughput_.end())
        pdcpThroughput_.insert(std::pair<unsigned int, throughput >(desc->nodeId_ , {0,0}));

}

void PacketFlowManagerEnb::insertRlcPdu(LogicalCid lcid, unsigned int rlcSno, SequenceNumberSet& pdcpSnoSet,  bool lastIsFrag)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertRlcPdu - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - Logical CID %d not present. It must be initialized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    if (desc->rlcSdusPerPdu_.find(rlcSno) != desc->rlcSdusPerPdu_.end())
        throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - RLC PDU SN %d already present for logical CID %d. Aborting", rlcSno, lcid);
    EV_FATAL << NOW << "node id "<< desc->nodeId_ << " PacketFlowManagerEnb::insertRlcPdu - Logical CID " << lcid << endl;

    SequenceNumberSet::iterator sit = pdcpSnoSet.begin();
    for (; sit != pdcpSnoSet.end(); ++sit)
    {
        unsigned int pdcpSno = *sit;
        if(desc->nodeId_ -1025 == 7)
                ww.record(pdcpSno);
        // store the RLC SDUs (PDCP PDUs) included in the RLC PDU
        desc->rlcSdusPerPdu_[rlcSno].insert(pdcpSno);

        // now store the inverse association, i.e., for each RLC SDU, record in which RLC PDU is included
        desc->rlcPdusPerSdu_[pdcpSno].insert(rlcSno);
        desc->rlcPdusPerSdu_[pdcpSno].insert(rlcSno);


        // set the PDCP entry time
        std::map<unsigned int, PdcpStatus>::iterator pit = desc->pdcpStatus_.find(pdcpSno);
        if(pit == desc->pdcpStatus_.end())
            throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - PdcpStatus for PDCP sno [%d] not present, this should not happen. Abort", pdcpSno);

        if(sit != pdcpSnoSet.end() && sit == --pdcpSnoSet.end()){
            pit->second.hasArrivedAll = !lastIsFrag;
//            desc->allPdcdSduToRlc_[pdcpSno] = !lastIsFrag;
        }else{
            pit->second.hasArrivedAll = true;
        }
        EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertRlcPdu - lcid[" << lcid << "], insert PDCP PDU " << pdcpSno << " in RLC PDU " << rlcSno << endl;
    }
}

void PacketFlowManagerEnb::insertRlcPdu(LogicalCid lcid, LteRlcUmDataPdu* rlcPdu, RlcBurstStatus status)
{
        std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
        if (cit == connectionMap_.end())
        {
            // this may occur after a handover, when data structures are cleared
            // EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertRlcPdu - Logical CID " << lcid << " not present." << endl;
            throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - Logical CID %d not present. It must be initialized before", lcid);
            return;
        }

        // get the descriptor for this connection
        StatusDescriptor* desc = &cit->second;
        EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertRlcPdu - Logical CID " << lcid << endl;

        unsigned int rlcSno = rlcPdu->getPduSequenceNumber();

        if (desc->rlcSdusPerPdu_.find(rlcSno) != desc->rlcSdusPerPdu_.end())
            throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - RLC PDU SN %d already present for logical CID %d. Aborting", rlcSno, lcid);

        FramingInfo fi = rlcPdu->getFramingInfo();

        RlcSduList* rlcSduList = rlcPdu->getRlcSudList();
        std::list<cPacket*>::const_iterator lit = rlcSduList->begin();
        LteRlcSdu* rlcSdu;
        FlowControlInfo* lteInfo;

        // manage burst
        if(status == START)
        {
            if(desc->burstState_ == true)
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . RLC burst status START incompatible with local status %d. Aborting", desc->nodeId_, lcid, desc->burstState_ );
            BurstStatus newBurst;
            newBurst.isComplited = false;
            newBurst.rlcPdu.insert(rlcSno);
            newBurst.startBurstTransmission = simTime();
            newBurst.burstSize += rlcPdu->getByteLength();
            desc->burstStatus_[++(desc->burstId_)] = newBurst;
            desc->burstState_ = true;
            EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertRlcPdu START"<<endl;

        }
        else if (status == STOP)
        {
            if(desc->burstState_ == false)
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . RLC burst status STOP incompatible with local status %d. Aborting", desc->nodeId_, lcid, desc->burstState_ );
            desc->burstStatus_[desc->burstId_].isComplited = true;
            desc->burstState_ = false;

            EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertRlcPdu STOP"<<endl;
        }
        else if(status == INACTIVE)
        {
            EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertRlcPdu INACTIVE"<<endl;

            if(desc->burstState_ == true)
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . RLC burst status INACTIVE incompatible with local status %d. Aborting", desc->nodeId_, lcid,  desc->burstState_);
        }
        else if(status == ACTIVE)
        {
            EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertRlcPdu ACTIVE"<<endl;

            if(desc->burstState_ == false)
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . RLC burst status ACTIVE incompatible with local status %d. Aborting", desc->nodeId_, lcid,  desc->burstState_ );
            std::map<BurstId, BurstStatus>::iterator bsit = desc->burstStatus_.find(desc->burstId_);
            if(bsit == desc->burstStatus_.end())
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . Burst status not found during active burst. Aborting", desc->nodeId_, lcid);
            bsit->second.rlcPdu.insert(rlcSno);
        }
        else
        {
         throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu RLCBurstStatus not recognized");
        }

        for (; lit != rlcSduList->end(); ++lit){
            rlcSdu = check_and_cast<LteRlcSdu*>(*lit);
            lteInfo = check_and_cast<FlowControlInfo*>(rlcSdu->getControlInfo());
            unsigned int pdcpSno = rlcSdu->getSnoMainPacket();

            if(desc->nodeId_ -1025 == 7)
                    ww.record(pdcpSno);

            // store the RLC SDUs (PDCP PDUs) included in the RLC PDU
            desc->rlcSdusPerPdu_[rlcSno].insert(pdcpSno);

            // now store the inverse association, i.e., for each RLC SDU, record in which RLC PDU is included
            desc->rlcPdusPerSdu_[pdcpSno].insert(rlcSno);
            desc->rlcPdusPerSdu_[pdcpSno].insert(rlcSno);


            // set the PDCP entry time
            std::map<unsigned int, PdcpStatus>::iterator pit = desc->pdcpStatus_.find(pdcpSno);
            if(pit == desc->pdcpStatus_.end())
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - PdcpStatus for PDCP sno [%d] not present, this should not happen. Abort", pdcpSno);

            if(lit != rlcSduList->end() && lit == --rlcSduList->end()){
                // 01 or 11, lsb 1 (3GPP TS 36.322)
                // means -> Last byte of the Data field does not correspond to the last byte of a RLC SDU.
                if(fi & 1 == 1)
                {
                    pit->second.hasArrivedAll = false;
                }
                else
                {
                    pit->second.hasArrivedAll = true;
                }

    //            desc->allPdcdSduToRlc_[pdcpSno] = !lastIsFrag;
            }
            else{
                pit->second.hasArrivedAll = true;
            }

            EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertRlcPdu - lcid[" << lcid << "], insert PDCP PDU " << pdcpSno << " in RLC PDU " << rlcSno << endl;
        }
}

void PacketFlowManagerEnb::discardRlcPdu(LogicalCid lcid, unsigned int rlcSno, bool fromMac)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::discardRlcPdu - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManagerEnb::discardRlcPdu - Logical CID %d not present. It must be initilized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    if (desc->rlcSdusPerPdu_.find(rlcSno) == desc->rlcSdusPerPdu_.end())
        throw cRuntimeError("PacketFlowManagerEnb::discardRlcPdu - RLC PDU SN %d not present for logical CID %d. Aborting", rlcSno, lcid);

    // get the PCDP SDUs fragmented in this RLC PDU
    SequenceNumberSet pdcpSnoSet = desc->rlcSdusPerPdu_.find(rlcSno)->second;
    SequenceNumberSet::iterator sit = pdcpSnoSet.begin();
    std::map<unsigned int, PdcpStatus>::iterator pit;
    std::map<unsigned int, SequenceNumberSet>::iterator rit;
    for (; sit != pdcpSnoSet.end(); ++sit)
    {
        unsigned int pdcpSno = *sit;

        rit = desc->rlcPdusPerSdu_.find(pdcpSno);
        if(rit == desc->rlcPdusPerSdu_.end())
            throw cRuntimeError("PacketFlowManagerEnb::discardRlcPdu - PdcpStatus for PDCP sno [%d] with lcid [%d] not present, this should not happen. Abort", pdcpSno, lcid);

        // remove the RLC PDUs that contains a fragment of this pdcpSno
        rit->second.erase(rlcSno);


        // set this pdcp sdu that a RLC has been discarded, i.e the arrived pdcp will be not entire.
        pit = desc->pdcpStatus_.find(pdcpSno);
        if(pit == desc->pdcpStatus_.end())
            throw cRuntimeError("PacketFlowManagerEnb::discardRlcPdu - PdcpStatus for PDCP sno [%d] already present, this should not happen. Abort", pdcpSno);

        if(fromMac)
            pit->second.discardedAtMac = true;
        else
            pit->second.discardedAtRlc = true;

        // if the set is empty AND
        // the pdcp pdu has been encapsulated all AND
        // a RLC referred to this PDCP has not been discarded at MAC (i.e 4NACKs) AND
        // a RLC referred to this PDCP has not been sent over the air
        // ---> all PDCP has been discarde at eNB before star its trasmission
        // compliant with ETSI 136 314 at 4.1.5.1

        if(rit->second.empty() && pit->second.hasArrivedAll && !pit->second.discardedAtMac && !pit->second.sentOverTheAir)
        {
            EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::discardRlcPdu - lcid[" << lcid << "], discarded PDCP PDU " << pdcpSno << " in RLC PDU " << rlcSno << endl;
            pktDiscardCounterPerUe_[desc->nodeId_] += 1;
            pktDiscardCounterTotal_ += 1;

        }
        // if the pdcp was entire and the set of rlc is empty, discard it
        if(rit->second.empty() && pit->second.hasArrivedAll){
            desc->rlcPdusPerSdu_.erase(pdcpSno);
        }
    }
    //remove discarded rlc pdu
    desc->rlcSdusPerPdu_.erase(rlcSno);
}

void PacketFlowManagerEnb::insertMacPdu(LogicalCid lcid, unsigned int macPduId, SequenceNumberSet& rlcSnoSet)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertMacPdu - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManagerEnb::insertMacPdu - Logical CID %d not present. It must be initilized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    if (desc->macSdusPerPdu_.find(macPduId) != desc->macSdusPerPdu_.end())
        throw cRuntimeError("PacketFlowManagerEnb::insertMacPdu - MAC PDU ID %d already present for logical CID %d. Aborting", macPduId, lcid);

    SequenceNumberSet::iterator sit = rlcSnoSet.begin();
    for (; sit != rlcSnoSet.end(); ++sit)
    {
        unsigned int rlcSno = *sit;

        // ??????
        // record the associaton RLC PDU - MAC PDU only if the RLC PDU contains at least one entire SDU
        // the condition holds if the RLC PDU SN is stored in the data structure rlcSdusPerPdu_


        std::map<unsigned int, SequenceNumberSet>::iterator tit = desc->rlcSdusPerPdu_.find(rlcSno);
        if(tit == desc->rlcSdusPerPdu_.end())
           throw cRuntimeError("PacketFlowManagerEnb::insertMacPdu - RLC PDU ID %d not present in the status descriptor of lcid %d ", rlcSno, lcid);

        // store the MAC SDUs (RLC PDUs) included in the MAC PDU
        desc->macSdusPerPdu_[macPduId].insert(rlcSno);
        EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::insertMacPdu - lcid[" << lcid << "], insert RLC PDU " << rlcSno << " in MAC PDU " << macPduId << endl;


        // set the pdcp pdus related to this RLC as sent over the air since this method is called after the MAC ID
        // has been inserted in the HARQBuffer
        SequenceNumberSet pdcpSet = tit->second;
        SequenceNumberSet::iterator pit = pdcpSet.begin();
        std::map<unsigned int, PdcpStatus>::iterator sdit;
        for (; pit != pdcpSet.end(); ++pit)
        {
            sdit = desc->pdcpStatus_.find(*pit);
            if(sdit == desc->pdcpStatus_.end())
                throw cRuntimeError("PacketFlowManagerEnb::insertMacPdu - PdcpStatus for PDCP sno [%d] not present, this should not happen. Abort", *pit);
            sdit->second.sentOverTheAir = true;
        }
    }
}

void PacketFlowManagerEnb::macPduArrived(LogicalCid lcid, unsigned int macPdu)
{


    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::notifyHarqProcess - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManagerEnb::insertMacPdu - Logical CID %d not present. It must be initilized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::macPduArrived - MAC PDU "<< macPdu << " of lcid " << lcid << " arrived." << endl;

    EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::macPduArrived - Get MAC PDU ID [" << macPdu << "], which contains:" << endl;

    // === STEP 1 ==================================================== //
    // === recover the set of RLC PDU SN from the above MAC PDU ID === //

//    unsigned int macPduId = desc->macPduPerProcess_[macPdu];
    unsigned int macPduId = macPdu;

    if (macPduId == 0)
    {
        EV << NOW << " PacketFlowManagerEnb::insertMacPdu - The process does not contain entire SDUs" << endl;
        return;
    }

//    desc->macPduPerProcess_[macPdu] = 0; // reset

    std::map<unsigned int, SequenceNumberSet>::iterator mit = desc->macSdusPerPdu_.find(macPduId);
    if (mit == desc->macSdusPerPdu_.end())
        throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - MAC PDU ID %d not present for logical CID %d. Aborting", macPduId, lcid);
    SequenceNumberSet rlcSnoSet = mit->second;

    // === STEP 2 ========================================================== //
    // === for each RLC PDU SN, recover the set of RLC SDU (PDCP PDU) SN === //

    SequenceNumberSet::iterator it = rlcSnoSet.begin();

    SequenceNumberSet pdcpTti; // in case there are many rlc per mac pdu, this set take trace of the tti counted per pdpc
    for (; it != rlcSnoSet.end(); ++it)
    {
        // for each RLC PDU
        unsigned int rlcPduSno = *it;

        EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::macPduArrived - --> RLC PDU [" << rlcPduSno << "], which contains:" << endl;

        std::map<unsigned int, SequenceNumberSet>::iterator nit = desc->rlcSdusPerPdu_.find(rlcPduSno);
        if (nit == desc->rlcSdusPerPdu_.end())
            throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - RLC PDU SN %d not present for logical CID %d. Aborting", rlcPduSno, lcid);
        SequenceNumberSet pdcpSnoSet = nit->second;


        // check end of a burst
        // for each burst_id we have to search if the relative set has the RLC
        // it has been assumed that the bursts are quickly emptied so the operation
        // is not computationally heavy
        std::map<BurstId, BurstStatus>::iterator bsit = desc->burstStatus_.begin();
        SequenceNumberSet::iterator rlcit;
        for(; bsit != desc->burstStatus_.end(); ++bsit)
        {
            rlcit =  bsit->second.rlcPdu.find(rlcPduSno);
            if(rlcit != bsit->second.rlcPdu.end())
            {
                bsit->second.rlcPdu.erase(rlcit);
                if(bsit->second.rlcPdu.empty() && bsit->second.isComplited)
                {
                    // compute throughput
                    throughputMap::iterator tit = pdcpThroughput_.find(desc->nodeId_);
                    if(tit == pdcpThroughput_.end())
                        throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - Node id %d is not in pdcp throughput map structure, this should not happen. Aborting", desc->nodeId_);
                    //the REAL count of the bytes goes here, based on ack and nack!
                    tit->second.pktSizeCount += bsit->second.burstSize;
                    tit->second.time += (simTime() - bsit->second.startBurstTransmission);
                    //times_[desc->nodeId_].record(bsit->second.burstSize/(simTime() - bsit->second.startBurstTransmission).dbl());
                    desc->burstStatus_.erase(bsit); // remove emptied burst
                 }
            }
        }

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

            EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::macPduArrived - ----> PDCP PDU [" << pdcpPduSno << "]" << endl;

            std::map<unsigned int, SequenceNumberSet>::iterator oit = desc->rlcPdusPerSdu_.find(pdcpPduSno);
            if (oit == desc->rlcPdusPerSdu_.end())
                throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - PDCP PDU SN %d not present for logical CID %d. Aborting", pdcpPduSno, lcid);

            // oit->second is the set of RLC PDU in which the PDCP PDU is contained

            // the RLC PDU SN must be present in the set
            SequenceNumberSet::iterator kt = oit->second.find(rlcPduSno);
            if (kt == oit->second.end())
                 throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - RLC PDU SN %d not present in the set of PDCP PDU SN %d for logical CID %d. Aborting", pdcpPduSno, rlcPduSno, lcid);

            // the RLC PDU has been sent, so erase it from the set
            oit->second.erase(kt);

            std::map<unsigned int, PdcpStatus>::iterator pit = desc->pdcpStatus_.find(pdcpPduSno);
            if(pit == desc->pdcpStatus_.end())
                throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - PdcpStatus for PDCP sno [%d] not present for lcid [%d], this should not happen. Abort", pdcpPduSno, lcid);



            // check whether the set is now empty
            if (desc->rlcPdusPerSdu_[pdcpPduSno].empty())
            {


                // set the time for pdcpPduSno
                if(pit->second.entryTime == 0)
                    throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - PDCP PDU SN %d of Lcid %d has not an entry time timestamp, this should not happen. Aborting", pdcpPduSno, lcid);

                if(pit->second.hasArrivedAll && !pit->second.discardedAtRlc && !pit->second.discardedAtMac)
                { // the whole current pdcp seqNum has been received
                    EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::macPduArrived - ----> PDCP PDU [" << pdcpPduSno << "] has been completely sent, remove from PDCP buffer" << endl;

                    delayMap::iterator dit = pdcpDelay_.find(desc->nodeId_);
                    if(dit == pdcpDelay_.end())
                        throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - Node id %d is not in pdcp delay map structure, this should not happen. Aborting", desc->nodeId_);

                    double time = (simTime() - pit->second.entryTime).dbl() ;
                    times_[desc->nodeId_].record(time);
                    EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::macPduArrived - PDCP PDU "<< pdcpPduSno << " of lcid " << lcid << " acknowledged. Delay time: " << time << "ms"<< endl;

//                    times_[desc->nodeId_].record(time);
                    dit->second.time += (simTime() - pit->second.entryTime);
                    dit->second.pktCount += 1;

                    myset.erase(pdcpPduSno);
                    if((desc->nodeId_ - 1025) == 7)
//                        ii.record(pdcpPduSno);
                    // update next sno
                    nextPdcpSno_ = pdcpPduSno+1;

                    // remove pdcp status

                    oit->second.clear();
                    desc->rlcPdusPerSdu_.erase(oit); // erase PDCP PDU SN
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

void PacketFlowManagerEnb::discardMacPdu(LogicalCid lcid, unsigned int macPduId)
{

    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::notifyHarqProcess - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManagerEnb::discardMacPdu - Logical CID %d not present. It must be initilized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::discardMacPdu - MAC PDU "<< macPduId << " of lcid " << lcid << " arrived." << endl;

    EV_FATAL << NOW << "node id "<< desc->nodeId_ -1025 << " PacketFlowManagerEnb::discardMacPdu - Get MAC PDU ID [" << macPduId << "], which contains:" << endl;

    // === STEP 1 ==================================================== //
    // === recover the set of RLC PDU SN from the above MAC PDU ID === //

//    unsigned int macPduId = desc->macPduPerProcess_[macPdu];

    if (macPduId == 0)
    {
        EV << NOW << " PacketFlowManagerEnb::discardMacPdu - The process does not contain entire SDUs" << endl;
        return;
    }

//    desc->macPduPerProcess_[macPdu] = 0; // reset

    std::map<unsigned int, SequenceNumberSet>::iterator mit = desc->macSdusPerPdu_.find(macPduId);
    if (mit == desc->macSdusPerPdu_.end())
        throw cRuntimeError("PacketFlowManagerEnb::discardMacPdu - MAC PDU ID %d not present for logical CID %d. Aborting", macPduId, lcid);
    SequenceNumberSet rlcSnoSet = mit->second;

    // === STEP 2 ========================================================== //
    // === for each RLC PDU SN, recover the set of RLC SDU (PDCP PDU) SN === //

    SequenceNumberSet::iterator it = rlcSnoSet.begin();
    for (; it != rlcSnoSet.end(); ++it)
    {
        discardRlcPdu(lcid, *it, true);

        // check end of a burst
        // for each burst_id we have to search if the relative set has the RLC
        // it has been assumed that the bursts are quickly emptied so the operation
        // is not computationally heavy
//        std::map<BurstId, BurstStatus>::iterator bsit = desc->burstStatus_.begin();
//        SequenceNumberSet::iterator rlcit;
//        for(; bsit != desc->burstStatus_.end(); ++bsit)
//        {
//            rlcit =  bsit->second.rlcPdu.find(rlcPduSno);
//            if(rlcit != bsit->second.rlcPdu.end())
//            {
//                bsit->second.rlcPdu.erase(rlcit);
//                if(bsit->second.rlcPdu.empty())
//                {
//                    // compute throughput
//                    throughputMap::iterator tit = pdcpThroughput_.find(desc->nodeId_);
//                    if(tit == pdcpThroughput_.end())
//                        throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - Node id %d is not in pdcp throughput map structure, this should not happen. Aborting", desc->nodeId_);
//                    //the count of the bytes goes here, based on ack and nack!
//                    desc->burstStatus_.erase(bsit); // remove emptied burst
//                 }

        }




}

void PacketFlowManagerEnb::resetDiscardCounterPerUe(MacNodeId id)
{
    std::map<MacNodeId, int>::iterator it = pktDiscardCounterPerUe_.find(id);
    if (it == pktDiscardCounterPerUe_.end())
    {
        // maybe it is possible? think about it
        // yes
//        throw cRuntimeError("PacketFlowManagerEnb::resetCounterPerUe - nodeId [%d] not present", id);
        return;
    }
    it->second = 0;
}

double PacketFlowManagerEnb::getDiscardedPktPerUe(MacNodeId id)
{
    std::map<MacNodeId, int>::iterator it = pktDiscardCounterPerUe_.find(id);
    if (it == pktDiscardCounterPerUe_.end())
    {
        // maybe it is possible? think about it
        // yes, if I do not discard anything
        //throw cRuntimeError("PacketFlowManagerEnb::getTotalDiscardedPckPerUe - nodeId [%d] not present", id);
        return 0;
    }
    return (double)it->second/pdcp_->getPktCountPerUe(id);
    }

double PacketFlowManagerEnb::getDiscardedPkt()
{
    return (double) pktDiscardCounterTotal_/pdcp_->getPktCount();
}

void PacketFlowManagerEnb::insertHarqProcess(LogicalCid lcid, unsigned int harqProcId, unsigned int macPduId)
{
//    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
//    if (cit == connectionMap_.end())
//    {
//        // this may occur after a handover, when data structures are cleared
//        EV << NOW << " PacketFlowManagerEnb::insertHarqProcess - Logical CID " << lcid << " not present." << endl;
//        return;
//    }
//
//    // get the descriptor for this connection
//    StatusDescriptor* desc = &cit->second;
//
//    // record the associaton MAC PDU - HARQ process only if the MAC PDU contains a RLC PDU that, in turn, contains at least one entire SDU
//    // the condition holds if the MAC PDU ID is stored in the data structure macSdusPerPdu_
//    if (desc->macSdusPerPdu_.find(macPduId) != desc->macSdusPerPdu_.end())
//    {
//        // store the MAC PDU ID included into the given HARQ process
//        desc->macPduPerProcess_[harqProcId] = macPduId;
//        EV << NOW << " PacketFlowManagerEnb::insertMacPdu - lcid[" << lcid << "], insert MAC PDU " << macPduId << " in HARQ PROCESS " << harqProcId << endl;
//    }
}

double PacketFlowManagerEnb::getDelayStatsPerUe(MacNodeId id)
{
    delayMap::iterator it = pdcpDelay_.find(id);
    if (it == pdcpDelay_.end())
    {
        // this may occur after a handover, when data structures are cleared
        EV_FATAL << NOW << " PacketFlowManagerEnb::getDelayStatsPerUe - Delay Stats for Node Id " << id << " not present." << endl;
        return 0;
    }

    double totalMs = (it->second.time.dbl())*1000; // ms
    double delayMean = totalMs / it->second.pktCount;
    return delayMean;
}

void PacketFlowManagerEnb::resetDelayCounterPerUe(MacNodeId id)
{
    delayMap::iterator it = pdcpDelay_.find(id);
    if (it == pdcpDelay_.end())
    {
       // this may occur after a handover, when data structures are cleared
       EV_FATAL << NOW << " PacketFlowManagerEnb::getDelayStatsPerUe - Delay Stats for Node Id " << id << " not present." << endl;
       return;
    }

    it->second = {0,0};

    //or
    // it->second.fisrt = 0
    // it->second.second = 0
}

double PacketFlowManagerEnb::getThroughputStatsPerUe(MacNodeId id)
{
    throughputMap::iterator it = pdcpThroughput_.find(id);
    if (it == pdcpThroughput_.end())
    {
        // this may occur after a handover, when data structures are cleared
        EV_FATAL << NOW << " PacketFlowManagerEnb::getThroughputStatsPerUe - Throughput Stats for Node Id " << id << " not present." << endl;
        return 0;
    }

    double time = (it->second.time.dbl()); // ms
    double throughput = it->second.pktSizeCount/time;
    return throughput;
}

void PacketFlowManagerEnb::resetThroughputCounterPerUe(MacNodeId id)
{
    throughputMap::iterator it = pdcpThroughput_.find(id);
    if (it == pdcpThroughput_.end())
    {
        EV_FATAL << NOW << " PacketFlowManagerEnb::resetThroughputCounterPerUe - Throughput Stats for Node Id " << id << " not present." << endl;
        return;
    }
    it->second = {0,0};
}


void PacketFlowManagerEnb::deleteUe(MacNodeId id)
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
//            lcidToNodeIdmap_.erase(it->first);
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

void PacketFlowManagerEnb::finish()
{
}
