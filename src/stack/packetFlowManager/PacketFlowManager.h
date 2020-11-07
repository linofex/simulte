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

// maybe it could mantain a reference to the collector

typedef std::set<unsigned int> SequenceNumberSet;


class EnodeBStatsCollector;
class UeStatsCollector;

/*
 * TODO description
 */
class PacketFlowManager : public cSimpleModule
{
    /*
     * A UE can have different active connections at the same time, hence we need to
     * maintain the status for each of them
     */
    typedef struct {
        std::map<unsigned int, SequenceNumberSet> rlcPdusPerSdu_;  // for each RLC SDU, stores the RLC PDUs where the former was fragmented
        std::map<unsigned int, SequenceNumberSet> rlcPdusPerSduDiscard_;  // for each RLC SDU, stores the RLC PDUs where the former was fragmented

        std::map<unsigned int, SequenceNumberSet> rlcSdusPerPdu_;  // for each RLC PDU, stores the included RLC SDUs
        std::map<unsigned int, SequenceNumberSet> macSdusPerPdu_;  // for each MAC PDU, stores the included MAC SDUs (should be a 1:1 association)
        std::vector<unsigned int> macPduPerProcess_;               // for each HARQ process, stores the included MAC PDU
    } StatusDescriptor;
    std::map<LogicalCid, StatusDescriptor> connectionMap_;
    std::map<LogicalCid, MacNodeId> lcidToNodeIdmap_;
    std::map<MacNodeId, int> pktDiscardCounterPerUe_; // discard count per user
    int pktDiscardCounterTotal_;

    EnodeBStatsCollector *eNBcollector_;  // fixed reference to the enodeb collector
    UeStatsCollector *ueCollector_;       // dynamic reference to the desired ueCollector

    // when a mode switch occurs, the PDCP and RLC entities should be informed about the next SN to use,
    // i.e., the first SN not transmitted due to the mode switch
    unsigned int nextPdcpSno_;
    unsigned int nextRlcSno_;

  protected:

    virtual int numInitStages() const { return 2; }
    virtual void initialize(int stage);

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

    void insertRlcPdu(LogicalCid lcid, unsigned int rlcSno, SequenceNumberSet& pdcpSnoSet);


    /*
     * This method is used to take trace of all discarded RLC pdus. If I discard
     * all the RLC pdus that compose a PCDP SDU, I have to notify this in order to
     * update the L2Meas Discard Rate
     */
    void discardRlcPdu(LogicalCid lcid, unsigned int rlcSno);

//    void insertMacPdu(LogicalCid lcid, unsigned int macPduId, SequenceNumberSet& rlcSnoSet);
//    void insertHarqProcess(LogicalCid lcid, unsigned int harqProcId, unsigned int macPduId);

    // invoked by the MAC layer to notify that harqProcId is completed.
    // This method need to go back up the chain of sequence numbers to identify which
    // PDCP SDUs have been transmitted in this process
    void notifyHarqProcess(LogicalCid lcid, unsigned int harqProcId);

    unsigned int getNextRlcSno() { return nextRlcSno_; }
    unsigned int getNextPdcpSno() { return nextPdcpSno_; }

    /*
     * I also need methods to notify the discarded packets both to the EnodeBCollector
     * and the single UE. Add them to the collectors
     */
    void notifyStats();
    void notifyStatsToUe(MacNodeId nodeId);
    void resetCounters();
    int getDiscartedPkt(MacNodeId nodeId);



};

#endif
