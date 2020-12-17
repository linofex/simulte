//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_PACKETFLOWMANAGERUE_H_
#define _LTE_PACKETFLOWMANAGERUE_H_

#include <omnetpp.h>
#include "common/LteCommon.h"
#include "PacketFlowManagerBase.h"





/*
 * This module is responsible for keep trace of all PDCP SDUs.
 * A PDCP SDU passes the following state while it is going down
 * through the LTE NIC layers:
 * 
 * PDCP SDU
 * few operations
 * PDCP PDU
 * RLC SDU
 * RLC PDU or fragmented in more then one RLC PDUs
 * MAC SDU
 * inserted into one TB
 * MAC PDU (aka TB)
 *
 * Each PDCP has its own seq number, managed by the corresponding LCID
 * 
 * The main functions of this module are:
 *  - detect PDCP SDU discarded (no part transmitted)
 *  - calculate the delay time of a pkt, from PDCP SDU to last Harq ACK of the
 *    corresponding seq number.  
 */

class LteRlcUmDataPdu;
class LtePdcpRrcUe;
class PacketFlowManagerUe : public PacketFlowManagerBase
{

        /*
        * The node can have different active connections (lcid) at the same time, hence we need to
        * maintain the status for each of them
        */
        typedef struct {
            MacNodeId nodeId_; // dest node of this lcid
            std::map<unsigned int, PdcpStatus> pdcpStatus_; // a pdcp pdu can be fragmented in many rlc that could be sent and ack in different time (this prevent early remove on ack)
            std::map<unsigned int, SequenceNumberSet> rlcPdusPerSdu_;  // for each RLC SDU, stores the RLC PDUs where the former was fragmented
            std::map<unsigned int, SequenceNumberSet> rlcSdusPerPdu_;  // for each RLC PDU, stores the included RLC SDUs
            std::map<unsigned int, SequenceNumberSet> macSdusPerPdu_;  // for each MAC PDU, stores the included MAC SDUs (should be a 1:1 association)
            std::vector<unsigned int> macPduPerProcess_;               // for each HARQ process, stores the included MAC PDU
        } StatusDescriptor;

        typedef  std::map<LogicalCid, StatusDescriptor> ConnectionMap;
        ConnectionMap connectionMap_; // lcid to the corresponding StatusDescriptor

    LtePdcpRrcUe *pdcp_;
    Delay pdcpDelay;

    //debug vars to be deleted
    cOutVector times_;
    std::set<unsigned int> myset;



    protected:

    virtual int numInitStages() const { return 2; }
    virtual void initialize(int stage);
    virtual void initPdcpStatus(StatusDescriptor* desc, unsigned int pdcp, unsigned int pdcpSize, simtime_t& arrivalTime);


//    bool hasFragments(LogicalCid lcid, unsigned int pdcp);

  public:
    PacketFlowManagerUe();
    // return true if a structure for this lcid is present
    virtual bool checkLcid(LogicalCid lcid);
    // initialize a new structure for this lcid
    virtual void initLcid(LogicalCid lcid, MacNodeId nodeId);
    // reset the structure for this lcid
    virtual void clearLcid(LogicalCid lcid);
    // reset structures for all connections
    virtual void clearAllLcid();


    /* 
    * This method insert a new pdcp seqnum and the corresponding entry time
    * @param lcid 
    * @param pdcpSno sequence number of the pdcp pdu
    * @param entryTime the time the packet enters PDCP layer
    */
    virtual void insertPdcpSdu(LogicalCid lcid, unsigned int pdcpSno,unsigned int sduSize, simtime_t entryTime);
    
    /* 
    * This method insert a new rlc seqnum and the corresponding pdcp pdus inside it
    * @param lcid 
    * @param rlcSno sequence number of the rlc pdu
    * @param pdcpSnoSet list of pdcp pdu inside the rlc pdu
    * @param lastIsFrag used to inform if the last pdcp is fragmented or not
    */
    virtual void insertRlcPdu(LogicalCid lcid, unsigned int rlcSno, SequenceNumberSet& pdcpSnoSet, bool lastIsFrag);
    
    /*
    * This method insert a new rlc seqnum and the corresponding pdcp pdus inside it
    * @param lcid
    * @param rlcPdu packet pointer
    */
    virtual void insertRlcPdu(LogicalCid lcid, LteRlcUmDataPdu* rlcPdu, RlcBurstStatus status);

    /* 
    * This method insert a new macPduId Omnet id and the corresponding rlc pdus inside it
    * @param lcid 
    * @param macPduId Omnet id of the mac pdu
    * @param rlcSnoSet list of rlc pdu inside the rlc pdu
    */  
    virtual void insertMacPdu(LogicalCid lcid, unsigned int macPduId, SequenceNumberSet& rlcSnoSet);

    /*
    * This method checks if the HARQ ack relative to a macPduId acknowledges an ENTIRE
    * pdcp sdu 
    * @param lcid
    * @param macPduId Omnet id of the mac pdu 
    */
    virtual void macPduArrived(LogicalCid lcid, unsigned int macPduId);

    /*
    * This method is called after maxHarqTrasmission of a MAC PDU ID has been
    * reached. The PDCP, RLC, sno referred to the macPdu are cleared from the
    * data structures
    * @param lcid
    * @param macPduId Omnet id of the mac pdu to be discarded
    */
    virtual void discardMacPdu(LogicalCid lcid, unsigned int macPduId);

    /*
     * This method is used to take trace of all discarded RLC pdus. If all rlc pdus
     * that compose a PCDP SDU have been discarded the discarded counters are updated
     * @param lcid
     * @param rlcSno sequence number of the rlc pdu
     * @param fromMac used when this method is called by discardMacPdu
     */
    virtual void discardRlcPdu(LogicalCid lcid, unsigned int rlcSno, bool fromMac = false);

    virtual void insertHarqProcess(LogicalCid lcid, unsigned int harqProcId, unsigned int macPduId);


    /*
     * invoked by the MAC layer to notify that harqProcId is completed.
     * This method need to go back up the chain of sequence numbers to identify which
     * PDCP SDUs have been transmitted in this process.
     */
    // void notifyHarqProcess(LogicalCid lcid, unsigned int harqProcId);

    /*
     * I also need methods that the PDCP layer calls when
     * it needs the discarded pkts, pkt delay and throughput.
     * The methods are called per specific UE
     */

    DiscardedPkts getDiscardedPkt();
    void resetDiscardPktCounter();

    double getDelayStats();
    void resetDelayCounter();

    virtual ~PacketFlowManagerUe();
    virtual void finish();

};
#endif
