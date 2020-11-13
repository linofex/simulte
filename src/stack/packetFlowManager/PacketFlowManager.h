//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_PACKETFLOWMANAGER_H_
#define _LTE_PACKETFLOWMANAGER_H_

#include <omnetpp.h>
#include "common/LteCommon.h"


typedef std::set<unsigned int> SequenceNumberSet;

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


class PacketFlowManager : public cSimpleModule
{
    /*
     * The node can have different active connections (lcid) at the same time, hence we need to
     * maintain the status for each of them
     */
    typedef struct {
        MacNodeId nodeId_; // dest node of this lcid
        std::map<unsigned int, bool> allPdcdSduToRlc_; // a pdcp pdu can be fragmented in many rlc that could be sent and ack in different time (this prevent early remove on ack)
        std::map<unsigned int, simtime_t> pdcpSduEntryTime_; // for each pdcp save the time when it enter the pdcp layer
        std::map<unsigned int, SequenceNumberSet> rlcPdusPerSdu_;  // for each RLC SDU, stores the RLC PDUs where the former was fragmented
        std::map<unsigned int, SequenceNumberSet> rlcPdusPerSduDiscard_;  // for each RLC SDU, stores the RLC PDUs where the former was fragmented used for discarding
        std::map<unsigned int, SequenceNumberSet> rlcSdusPerPdu_;  // for each RLC PDU, stores the included RLC SDUs
        std::map<unsigned int, SequenceNumberSet> macSdusPerPdu_;  // for each MAC PDU, stores the included MAC SDUs (should be a 1:1 association)
        std::vector<unsigned int> macPduPerProcess_;               // for each HARQ process, stores the included MAC PDU
    } StatusDescriptor;

    std::map<LogicalCid, StatusDescriptor> connectionMap_; // lcid to the corresponding StatusDescriptor
    std::map<LogicalCid, MacNodeId> lcidToNodeIdmap_;      // lcid to dest NodeId
    std::map<MacNodeId, int> pktDiscardCounterPerUe_;      // discard counter per NodeId (UE)
    int pktDiscardCounterTotal_; // total discard counter
    typedef std::map<MacNodeId, std::pair< simtime_t, unsigned int >> delayMap;
    delayMap pdcpDelay_; // map that sums all the delay times of a dest NodeId (UE) and the corresponding counter 

    LteNodeType nodeType_; // UE or ENODED (used for set MACROS)
    short int harqProcesses_; // number of harq processes


    // when a mode switch occurs, the PDCP and RLC entities should be informed about the next SN to use,
    // i.e., the first SN not transmitted due to the mode switch
    unsigned int nextPdcpSno_;
    unsigned int nextRlcSno_;

    std::map<MacNodeId,cOutVector> times_;

  protected:

    virtual int numInitStages() const { return 2; }
    virtual void initialize(int stage);

    bool hasFragments(LogicalCid lcid, unsigned int pdcp);

  public:
    PacketFlowManager();
    // return true if a structure for this lcid is present
    bool checkLcid(LogicalCid lcid);
    // initialize a new structure for this lcid
    void initLcid(LogicalCid lcid, MacNodeId nodeId);
    // reset the structure for this lcid
    void clearLcid(LogicalCid lcid);
    // reset structures for all connections
    void clearAllLcid();

    /* 
    * This method insert a new pdcp seqnum and the corresponding entry time
    * @param lcid 
    * @param pdcpSno sequence number of the pdcp pdu
    * @param entryTime the time the packet enters PDCP layer
    */
    void insertPdcpSdu(LogicalCid lcid, unsigned int pdcpSno, simtime_t entryTime);
    
    /* 
    * This method insert a new rlc seqnum and the corresponding pdcp pdus inside it
    * @param lcid 
    * @param rlcSno sequence number of the rlc pdu
    * @param pdcpSnoSet list of pdcp pdu inside the rlc pdu
    * @param lastIsFrag used to inform if the last pdcp is fragmented or not
    */
    void insertRlcPdu(LogicalCid lcid, unsigned int rlcSno, SequenceNumberSet& pdcpSnoSet, bool lastIsFrag);
    
    /* 
    * This method insert a new macPduId Omnet id and the corresponding rlc pdus inside it
    * @param lcid 
    * @param macPduId Omnet id of the mac pdu
    * @param rlcSnoSet list of rlc pdu inside the rlc pdu
    */  
    void insertMacPdu(LogicalCid lcid, unsigned int macPduId, SequenceNumberSet& rlcSnoSet);

    /*
    * This method checks if the HARQ ack relative to a macPduId acknowledges an ENTIRE
    * pdcp sdu 
    * @param lcid
    * @param macPduId Omnet id of the mac pdu 
    */
    void macPduArrived(LogicalCid lcid, unsigned int macPduId);

    /*
     * This method is used to take trace of all discarded RLC pdus. If all rlc pdus
     * that compose a PCDP SDU have been discarded the discarded counters are updated
     * @param lcid
     * @parm rlcSno sequence number of the rlc pdu
     */
    void discardRlcPdu(LogicalCid lcid, unsigned int rlcSno);

//    void insertHarqProcess(LogicalCid lcid, unsigned int harqProcId, unsigned int macPduId);

    // invoked by the MAC layer to notify that harqProcId is completed.
    // This method need to go back up the chain of sequence numbers to identify which
    // PDCP SDUs have been transmitted in this process
    // void notifyHarqProcess(LogicalCid lcid, unsigned int harqProcId);

    unsigned int getNextRlcSno() { return nextRlcSno_; }
    unsigned int getNextPdcpSno() { return nextPdcpSno_; }

    /*
     * I also need methods that the PDCP layer calls when
     * it needs the discarded pkts and pkt delay.
     * The methods are called per specific UE
     */

    int getTotalDiscardedPckPerUe(MacNodeId id);
    int getTotalDiscardedPck();
    void resetCounter();
    void resetCounterPerUe(MacNodeId id);

    double getDelayStatsPerUe(MacNodeId);
    void resetDelayCounterPerUe(MacNodeId id);
    void deleteUe(MacNodeId id);

};
#endif
