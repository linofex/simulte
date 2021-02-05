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
    //debug variables
    ee.setName("ee");
    ii.setName("ii");
    ww.setName("iii");

}

PacketFlowManagerEnb::~PacketFlowManagerEnb()
{
    connectionMap_.clear();
    pktDiscardCounterPerUe_.clear();
    pdcpDelay_.clear();
    pdcpThroughput_.clear();
}

void PacketFlowManagerEnb::initialize(int stage)
{
    if (stage == 1)
    {
        PacketFlowManagerBase::initialize(stage);
        pdcp_ = check_and_cast<LtePdcpRrcEnb *>(getParentModule()->getSubmodule("pdcpRrc"));
        headerCompressedSize_ = pdcp_->par("headerCompressedSize");
        if (headerCompressedSize_ == -1)
            headerCompressedSize_ = 0;
        //delaySignal = registerSignal("delay");
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
    //newDesc.macPduPerProcess_.resize(harqProcesses_, 0);

    BurstStatus newBurstStatus;
    newBurstStatus.burstSize = 0;
    newBurstStatus.rlcPdu.clear();
    newBurstStatus.startBurstTransmission = -1;
    newDesc.burstStatus_.clear();

    //debug
    std::stringstream timeStream;
    timeStream << nodeId -1025<< "time";
    std::string timeString = timeStream.str();
    char* timeChar = (char*) timeString.c_str();
    //times_[nodeId].setName(timeChar);
//    std::stringstream tPutStream;
//    tPutStream << nodeId -1025<< "tput";
//    std::string tPutString = tPutStream.str();
//    char* tPutChar = (char*) tPutString.c_str();
//    tput_[nodeId].setName(tPutChar);


    connectionMap_[lcid] = newDesc;
    pktDiscardCounterPerUe_[nodeId] = 0;
    EV_FATAL << NOW << " node id "<< nodeId << " PacketFlowManagerEnb::initLcid - initialized lcid " << lcid << endl;
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


//        for (int i=0; i<harqProcesses_; i++)
            //connectionMap_[lcid].macPduPerProcess_[i] = 0;
    }

    EV_FATAL << NOW << " node id "<< connectionMap_[lcid].nodeId_ << " PacketFlowManagerEnb::clearLcid - cleared data structures for lcid " << lcid << endl;
}

void PacketFlowManagerEnb::clearAllLcid()
{
    connectionMap_.clear();
    EV_FATAL << NOW << " PacketFlowManagerEnb::clearAllLcid - cleared data structures for all lcids "<< endl;
}

void PacketFlowManagerEnb::initPdcpStatus(StatusDescriptor* desc, unsigned int pdcp, unsigned int sduHeaderSize, simtime_t& arrivalTime)
{
    // if pdcpStatus_ already present, error
    std::map<unsigned int, PdcpStatus>::iterator it = desc->pdcpStatus_.find(pdcp);
    if(it != desc->pdcpStatus_.end())
        throw cRuntimeError("PacketFlowManagerEnb::initPdcpStatus - PdcpStatus for PDCP sno [%d] already present for node %d, this should not happen. Abort", pdcp, desc->nodeId_);

    PdcpStatus newpdcpStatus;

    newpdcpStatus.entryTime = arrivalTime;
    newpdcpStatus.discardedAtMac = false;
    newpdcpStatus.discardedAtRlc = false;
    newpdcpStatus.hasArrivedAll  = false;
    newpdcpStatus.sentOverTheAir =  false;
    newpdcpStatus.pdcpSduSize = sduHeaderSize; // ************************* pdcpSduSize è headesize!!!
    newpdcpStatus.entryTime = arrivalTime;
    desc->pdcpStatus_[pdcp] = newpdcpStatus;
    EV_FATAL << "PacketFlowManagerEnb::initPdcpStatus - PDCP PDU " << pdcp << "  with header size " << sduHeaderSize << " added" << endl;
}

void PacketFlowManagerEnb::insertPdcpSdu(LogicalCid lcid, unsigned int pdcpSno,unsigned int sduSize, simtime_t entryTime)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManagerEnb::insertPdcpSdu - Logical CID %d not present. It must be initialized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;

    initPdcpStatus(desc, pdcpSno, sduSize, entryTime);
    EV_FATAL << NOW << " node id "<< desc->nodeId_<<" PacketFlowManagerEnb::insertPdcpSdu - PDCP status for PDCP PDU SN " << pdcpSno<<" added. Logical cid " << lcid << endl;


    // add user to delay time map if non already present since many lcids can belong to a one nodeId (UE)
    // consider to add at run time in case it is needed
    // put here for debug purposes
    if(pdcpDelay_.find(desc->nodeId_) == pdcpDelay_.end())
        pdcpDelay_.insert(std::pair<unsigned int, Delay>(desc->nodeId_ , {0,0}));

    if(pdcpThroughput_.find(desc->nodeId_) == pdcpThroughput_.end())
        pdcpThroughput_.insert(std::pair<unsigned int, Throughput >(desc->nodeId_ , {0,0}));

}

void PacketFlowManagerEnb::insertRlcPdu(LogicalCid lcid, unsigned int rlcSno, SequenceNumberSet& pdcpSnoSet,  bool lastIsFrag)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - Logical CID %d not present. It must be initialized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    if (desc->rlcSdusPerPdu_.find(rlcSno) != desc->rlcSdusPerPdu_.end())
        throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - RLC PDU SN %d already present for logical CID %d. Aborting", rlcSno, lcid);
    EV_FATAL << NOW << " node id "<< desc->nodeId_ << " PacketFlowManagerEnb::insertRlcPdu - Logical CID " << lcid << endl;

    SequenceNumberSet::iterator sit = pdcpSnoSet.begin();
    for (; sit != pdcpSnoSet.end(); ++sit)
    {
        unsigned int pdcpSno = *sit;

        // store the RLC SDUs (PDCP PDUs) included in the RLC PDU
        desc->rlcSdusPerPdu_[rlcSno].insert(pdcpSno);

        // now store the inverse association, i.e., for each RLC SDU, record in which RLC PDU is included
        desc->rlcPdusPerSdu_[pdcpSno].insert(rlcSno);

        std::map<unsigned int, PdcpStatus>::iterator pit = desc->pdcpStatus_.find(pdcpSno);
        if(pit == desc->pdcpStatus_.end())
            throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - PdcpStatus for PDCP sno [%d] not present, this should not happen. Abort", pdcpSno);

        // check if pdcp has been transmitted to Mac layer
        if(sit != pdcpSnoSet.end() && sit == --pdcpSnoSet.end()){
            pit->second.hasArrivedAll = !lastIsFrag;
        }else{
            pit->second.hasArrivedAll = true;
        }
        EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu - lcid[" << lcid << "], insert PDCP PDU " << pdcpSno << " in RLC PDU " << rlcSno << endl;
    }
}

void PacketFlowManagerEnb::insertRlcPdu(LogicalCid lcid, LteRlcUmDataPdu* rlcPdu, RlcBurstStatus status)
{
        std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
        if (cit == connectionMap_.end())
        {
            // this may occur after a handover, when data structures are cleared
            // EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu - Logical CID " << lcid << " not present." << endl;
            throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - Logical CID %d not present. It must be initialized before", lcid);
            return;
        }

        // get the descriptor for this connection
        StatusDescriptor* desc = &cit->second;
        EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu - Logical CID " << lcid << endl;

        unsigned int rlcSno = rlcPdu->getPduSequenceNumber();

        if (desc->rlcSdusPerPdu_.find(rlcSno) != desc->rlcSdusPerPdu_.end())
            throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - RLC PDU SN %d already present for logical CID %d. Aborting", rlcSno, lcid);

        FramingInfo fi = rlcPdu->getFramingInfo();
        unsigned numSdu = rlcPdu->getNumSdu();
        RlcSduList* rlcSduList = rlcPdu->getRlcSudList();
        std::list<cPacket*>::const_iterator lit = rlcSduList->begin();
        LteRlcSdu* rlcSdu;
        FlowControlInfo* lteInfo;

        // manage burst state, for debugging and avoid errors between rlc state and packetflowmanager state
        if(status == START)
        {
            if(desc->burstState_ == true)
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . RLC burst status START incompatible with local status %d. Aborting", desc->nodeId_, lcid, desc->burstState_ );
            BurstStatus newBurst;
            newBurst.isComplited = false;
//          newBurst.rlcPdu.insert(rlcSno);
            newBurst.startBurstTransmission = simTime();
            newBurst.burstSize = 0;
            desc->burstStatus_[++(desc->burstId_)] = newBurst;
            desc->burstState_ = true;
            EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu START burst " << desc->burstId_<< " at: " <<newBurst.startBurstTransmission<< endl;

        }
        else if (status == STOP)
        {
            if(desc->burstState_ == false)
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . RLC burst status STOP incompatible with local status %d. Aborting", desc->nodeId_, lcid, desc->burstState_ );
            desc->burstStatus_[desc->burstId_].isComplited = true;
            desc->burstState_ = false;
            EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu STOP burst " << desc->burstId_<< " at: " << simTime()<< endl;
        }
        else if(status == INACTIVE)
        {
            if(desc->burstState_ == true)
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . RLC burst status INACTIVE incompatible with local status %d. Aborting", desc->nodeId_, lcid,  desc->burstState_);
            EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu INACTIVE burst" << endl;

        }
        else if(status == ACTIVE)
        {
            if(desc->burstState_ == false)
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . RLC burst status ACTIVE incompatible with local status %d. Aborting", desc->nodeId_, lcid,  desc->burstState_ );
            std::map<BurstId, BurstStatus>::iterator bsit = desc->burstStatus_.find(desc->burstId_);
            if(bsit == desc->burstStatus_.end())
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . Burst status not found during active burst. Aborting", desc->nodeId_, lcid);
//            bsit->second.rlcPdu.insert(rlcSno);
            EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu ACTIVE burst " << desc->burstId_<<  endl;
        }
        else
        {
         throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu RLCBurstStatus not recognized");
        }

        std::map<BurstId, BurstStatus>::iterator bsit;
        int i = 0;
        int totalPdcpSduSize = 0;
        for (; lit != rlcSduList->end(); ++lit){
            rlcSdu = check_and_cast<LteRlcSdu*>(*lit);
            lteInfo = check_and_cast<FlowControlInfo*>(rlcSdu->getControlInfo());
            unsigned int pdcpSno = rlcSdu->getSnoMainPacket();


            // store the RLC SDUs (PDCP PDUs) included in the RLC PDU
            desc->rlcSdusPerPdu_[rlcSno].insert(pdcpSno);

            // now store the inverse association, i.e., for each RLC SDU, record in which RLC PDU is included
            desc->rlcPdusPerSdu_[pdcpSno].insert(rlcSno);

            // set the PDCP entry time
            std::map<unsigned int, PdcpStatus>::iterator pit = desc->pdcpStatus_.find(pdcpSno);
            if(pit == desc->pdcpStatus_.end())
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - PdcpStatus for PDCP sno [%d] not present, this should not happen. Abort", pdcpSno);

            if(lit != rlcSduList->end() && lit == --rlcSduList->end()){
                // 01 or 11, lsb 1 (3GPP TS 36.322)
                // means -> Last byte of the Data field does not correspond to the last byte of a RLC SDU.
                if((fi & 1) == 1)
                {
                    pit->second.hasArrivedAll = false;
                }
                else
                {
                    pit->second.hasArrivedAll = true;
                }
            }
            // since it is not the last part of the rlc, this pdcp has been entirely inserted in RLCs
            else{
                pit->second.hasArrivedAll = true;
            }

            if(status == ACTIVE || status == START)
            {
                if(i == 0 && (fi == 3 || fi == 2)) // first sdu is a fragment (no header)
                    totalPdcpSduSize += rlcSdu->getBitLength();
                else if(i == 0 && (fi == 0 || fi == 1)) // remove pdcp header, and adjust the size of the pdcp sdu
                    totalPdcpSduSize += (rlcSdu->getBitLength() - (lteInfo->getRlcType() == UM ? PDCP_HEADER_UM : PDCP_HEADER_AM) - headerCompressedSize_ + pit->second.pdcpSduSize);
                else if( i > 0 ) // the following are pdcp with header
                    totalPdcpSduSize += (rlcSdu->getBitLength() - (lteInfo->getRlcType() == UM ? PDCP_HEADER_UM : PDCP_HEADER_AM) - headerCompressedSize_ + pit->second.pdcpSduSize);
                i++;
            }

            EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu - lcid[" << lcid << "], insert PDCP PDU " << pdcpSno << " in RLC PDU " << rlcSno << endl;
        }

        if(status == ACTIVE || status == START)
        {
            std::map<BurstId, BurstStatus>::iterator bsit = desc->burstStatus_.find(desc->burstId_);
            if(bsit == desc->burstStatus_.end())
                throw cRuntimeError("PacketFlowManagerEnb::insertRlcPdu - node %d and lcid %d . Burst status not found during active burst. Aborting", desc->nodeId_, lcid);
            // add rlc to rlc set of the burst and the size
            EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertRlcPdu - lcid[" << lcid << "], insert RLC PDU of size " << totalPdcpSduSize <<endl;
            bsit->second.rlcPdu[rlcSno] = totalPdcpSduSize;
        }

}

void PacketFlowManagerEnb::discardRlcPdu(LogicalCid lcid, unsigned int rlcSno, bool fromMac)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::discardRlcPdu - Logical CID " << lcid << " not present." << endl;
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

        // find sdu -> rlcs for this pdcp
        rit = desc->rlcPdusPerSdu_.find(pdcpSno);
        if(rit == desc->rlcPdusPerSdu_.end())
            throw cRuntimeError("PacketFlowManagerEnb::discardRlcPdu - PdcpStatus for PDCP sno [%d] with lcid [%d] not present, this should not happen. Abort", pdcpSno, lcid);

        // remove the RLC PDUs that contains a fragment of this pdcpSno
        rit->second.erase(rlcSno);


        // set this pdcp sdu as discarded, flag use in macPduArrive to no take in account this pdcp
        pit = desc->pdcpStatus_.find(pdcpSno);
        if(pit == desc->pdcpStatus_.end())
            throw cRuntimeError("PacketFlowManagerEnb::discardRlcPdu - PdcpStatus for PDCP sno [%d] already present, this should not happen. Abort", pdcpSno);

        if(fromMac)
            pit->second.discardedAtMac = true; // discarded rate stats also
        else
            pit->second.discardedAtRlc = true;


        // if the set is empty AND
        // the pdcp pdu has been encapsulated all AND
        // a RLC referred to this PDCP has not been discarded at MAC (i.e max num max NACKs) AND
        // a RLC referred to this PDCP has not been sent over the air
        // ---> all PDCP has been discarded at eNB before star its transmission
        // count it in discarded stats
        // compliant with ETSI 136 314 at 4.1.5.1

        if(rit->second.empty() && pit->second.hasArrivedAll && !pit->second.discardedAtMac && !pit->second.sentOverTheAir)
        {
            EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::discardRlcPdu - lcid[" << lcid << "], discarded PDCP PDU " << pdcpSno << " in RLC PDU " << rlcSno << endl;
            pktDiscardCounterPerUe_[desc->nodeId_] += 1;
            pktDiscardCounterTotal_ += 1;

        }
        // if the pdcp was entire and the set of rlc is empty, discard it
        if(rit->second.empty() && pit->second.hasArrivedAll){
            desc->rlcPdusPerSdu_.erase(rit);
            //remove pdcp status
            desc->pdcpStatus_.erase(pit);
        }
    }
    removePdcpBurstRLC(desc, rlcSno, false);
    //remove discarded rlc pdu
    desc->rlcSdusPerPdu_.erase(rlcSno);
}

void PacketFlowManagerEnb::insertMacPdu(LogicalCid lcid, unsigned int macPduId, SequenceNumberSet& rlcSnoSet)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertMacPdu - Logical CID " << lcid << " not present." << endl;
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

        std::map<unsigned int, SequenceNumberSet>::iterator tit = desc->rlcSdusPerPdu_.find(rlcSno);
        if(tit == desc->rlcSdusPerPdu_.end())
           throw cRuntimeError("PacketFlowManagerEnb::insertMacPdu - RLC PDU ID %d not present in the status descriptor of lcid %d ", rlcSno, lcid);

        // store the MAC SDUs (RLC PDUs) included in the MAC PDU
        desc->macSdusPerPdu_[macPduId].insert(rlcSno);
        EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::insertMacPdu - lcid[" << lcid << "], insert RLC PDU " << rlcSno << " in MAC PDU " << macPduId << endl;


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

void PacketFlowManagerEnb::macPduArrived(LogicalCid lcid, unsigned int macPduId)
{
    std::map<LogicalCid, StatusDescriptor>::iterator cit = connectionMap_.find(lcid);
    if (cit == connectionMap_.end())
    {
        // this may occur after a handover, when data structures are cleared
        // EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::notifyHarqProcess - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManagerEnb::insertMacPdu - Logical CID %d not present. It must be initilized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::macPduArrived - MAC PDU "<< macPduId << " of lcid " << lcid << " arrived." << endl;
    EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::macPduArrived - Get MAC PDU ID [" << macPduId << "], which contains:" << endl;

    // === STEP 1 ==================================================== //
    // === recover the set of RLC PDU SN from the above MAC PDU ID === //

//    unsigned int macPduId = desc->macPduPerProcess_[macPdu];

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

    for (; it != rlcSnoSet.end(); ++it)
    {
        // for each RLC PDU
        unsigned int rlcPduSno = *it;

        EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::macPduArrived - --> RLC PDU [" << rlcPduSno << "], which contains:" << endl;

        std::map<unsigned int, SequenceNumberSet>::iterator nit = desc->rlcSdusPerPdu_.find(rlcPduSno);
        if (nit == desc->rlcSdusPerPdu_.end())
            throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - RLC PDU SN %d not present for logical CID %d. Aborting", rlcPduSno, lcid);
        SequenceNumberSet pdcpSnoSet = nit->second;

        // === STEP 3 ============================================================================ //
        // === (PDCP PDU) SN, recover the set of RLC PDU where it is included,                 === //
        // === remove the above RLC PDU SN. If the set becomes empty, compute the delay if     === //
        // === all PDCP PDU fragments have been transmitted                                    === //
        
        SequenceNumberSet::iterator jt = pdcpSnoSet.begin();
        for (; jt != pdcpSnoSet.end(); ++jt)
        {
            // for each RLC SDU (PDCP PDU), get the set of RLC PDUs where it is included
            unsigned int pdcpPduSno = *jt;

            EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::macPduArrived - ----> PDCP PDU [" << pdcpPduSno << "]" << endl;

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
                { // the whole current pdcp seqNum has been received by the UE
                    EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::macPduArrived - ----> PDCP PDU [" << pdcpPduSno << "] has been completely sent, remove from PDCP buffer" << endl;

//                    removePdcpBurst(desc, pit->second, pdcpPduSno, true); // check if the pdcp is part of a burst

                    delayMap::iterator dit = pdcpDelay_.find(desc->nodeId_);
                    if(dit == pdcpDelay_.end())
                        throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - Node id %d is not in pdcp delay map structure, this should not happen. Aborting", desc->nodeId_);

                    double time = (simTime() - pit->second.entryTime).dbl() ;
                    //times_[desc->nodeId_].record(time);

                    EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::macPduArrived - PDCP PDU "<< pdcpPduSno << " of lcid " << lcid << " acknowledged. Delay time: " << time << "ms"<< endl;

                    dit->second.time += (simTime() - pit->second.entryTime);
                    dit->second.pktCount += 1;

                    myset.erase(pdcpPduSno);

                        // update next sno
                    nextPdcpSno_ = pdcpPduSno+1;

                    // remove pdcp status
                    desc->pdcpStatus_.erase(pit);
                    oit->second.clear();
                    desc->rlcPdusPerSdu_.erase(oit); // erase PDCP PDU SN
                }
            }

       }
        nit->second.clear();
        desc->rlcSdusPerPdu_.erase(nit); // erase RLC PDU SN
        // update next sno
        nextRlcSno_ = rlcPduSno+1;
        removePdcpBurstRLC(desc, rlcPduSno, true); // check if the pdcp is part of a burst
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
        // EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::notifyHarqProcess - Logical CID " << lcid << " not present." << endl;
        throw cRuntimeError("PacketFlowManagerEnb::discardMacPdu - Logical CID %d not present. It must be initilized before", lcid);
        return;
    }

    // get the descriptor for this connection
    StatusDescriptor* desc = &cit->second;
    EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::discardMacPdu - MAC PDU "<< macPduId << " of lcid " << lcid << " arrived." << endl;

    EV_FATAL << NOW << " node id "<< desc->nodeId_<< " PacketFlowManagerEnb::discardMacPdu - Get MAC PDU ID [" << macPduId << "], which contains:" << endl;

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
    }
}

void PacketFlowManagerEnb::removePdcpBurst(StatusDescriptor* desc, PdcpStatus& pdcpStatus,  unsigned int pdcpSno, bool ack)
{
    // check end of a burst
    // for each burst_id we have to search if the relative set has the RLC
    // it has been assumed that the bursts are quickly emptied so the operation
    // is not computationally heavy
    // the other solution is to create <rlcPdu, burst_id> map
//    std::map<BurstId, BurstStatus>::iterator bsit = desc->burstStatus_.begin();
//    SequenceNumberSet::iterator rlcpit;
//    for(; bsit != desc->burstStatus_.end(); ++bsit)
//    {
//        rlcpit =  bsit->second.rlcPdu.find(pdcpSno);
//        if(rlcpit != bsit->second.rlcPdu.end())
//        {
//            if(ack == true)
//            {
//            // if arrived, sum it to the thpVolDl
//            bsit->second.burstSize += pdcpStatus.pdcpSduSize;
//            }
//            bsit->second.rlcPdu.erase(rlcpit);
//            if(bsit->second.rlcPdu.empty() && bsit->second.isComplited)
//            {
//                // compute throughput
//                throughputMap::iterator tit = pdcpThroughput_.find(desc->nodeId_);
//                if(tit == pdcpThroughput_.end())
//                    throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - Node id %d is not in pdcp throughput map structure, this should not happen. Aborting", desc->nodeId_);
//                tit->second.pktSizeCount += bsit->second.burstSize;
//                tit->second.time += (simTime() - bsit->second.startBurstTransmission);
//                double tp = bsit->second.burstSize/(simTime() - bsit->second.startBurstTransmission).dbl();
//                tput_[desc->nodeId_].record(tp);
//                EV_FATAL << NOW << " node id "<< desc->nodeId_ - 1025 << " PacketFlowManagerEnb::removePdcpBurst Burst "<< bsit->first << " length " << simTime() - bsit->second.startBurstTransmission<< " tput: "<< tp << endl;
//                desc->burstStatus_.erase(bsit); // remove emptied burst
//             }
//            break;
//        }
//    }
}



void PacketFlowManagerEnb::removePdcpBurstRLC(StatusDescriptor* desc, unsigned int rlcSno, bool ack)
{
    // check end of a burst
    // for each burst_id we have to search if the relative set has the RLC
    // it has been assumed that the bursts are quickly emptied so the operation
    // is not computationally heavy
    // the other solution is to create <rlcPdu, burst_id> map
    std::map<BurstId, BurstStatus>::iterator bsit = desc->burstStatus_.begin();
    std::map<unsigned int, unsigned int>::iterator rlcpit;
    for(; bsit != desc->burstStatus_.end(); ++bsit)
    {
        rlcpit =  bsit->second.rlcPdu.find(rlcSno);
        if(rlcpit != bsit->second.rlcPdu.end())
        {
            if(ack == true)
            {
            // if arrived, sum it to the thpVolDl
            bsit->second.burstSize += rlcpit->second;
            }
            bsit->second.rlcPdu.erase(rlcpit);
            if(bsit->second.rlcPdu.empty() && bsit->second.isComplited)
            {
                // compute throughput
                throughputMap::iterator tit = pdcpThroughput_.find(desc->nodeId_);
                if(tit == pdcpThroughput_.end())
                    throw cRuntimeError("PacketFlowManagerEnb::macPduArrived - Node id %d is not in pdcp throughput map structure, this should not happen. Aborting", desc->nodeId_);
                tit->second.pktSizeCount += bsit->second.burstSize;
                tit->second.time += (simTime() - bsit->second.startBurstTransmission);
                double tp = bsit->second.burstSize/(simTime() - bsit->second.startBurstTransmission).dbl();
               // tput_[desc->nodeId_].record(tp);

                EV_FATAL << NOW << " node id "<< desc->nodeId_ - 1025 << " PacketFlowManagerEnb::removePdcpBurst Burst "<< bsit->first << " length " << simTime() - bsit->second.startBurstTransmission<< "s, with size " << bsit->second.burstSize <<"bits -> tput: "<< tp <<" b/s" <<endl;
                desc->burstStatus_.erase(bsit); // remove emptied burst
             }
            break;
        }
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
    return (double)(it->second * 1000000)/pdcp_->getPktCountPerUe(id);
    }

double PacketFlowManagerEnb::getDiscardedPkt()
{
    return (double) (pktDiscardCounterTotal_ * 1000000)/pdcp_->getPktCount();
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

//    if(it->second.pktSizeCount == 0) // a burst is not finished yet, return a tput of this period

    double time = (it->second.time.dbl()*1000); // ms
    double throughput = (it->second.pktSizeCount)/time;
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

    throughputMap::iterator tit = pdcpThroughput_.find(id);
    if(tit != pdcpThroughput_.end())
        pdcpThroughput_.erase(tit);
}

void PacketFlowManagerEnb::finish()
{
}
