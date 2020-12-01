//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_PACKETFLOWMANAGERBASE_H_
#define _LTE_PACKETFLOWMANAGERBASE_H_

#include <omnetpp.h>
#include "common/LteCommon.h"
#include "common/MecCommon.h"


typedef std::set<unsigned int> SequenceNumberSet;
typedef unsigned int BurstId;

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
struct StatusDescriptor;

class PacketFlowManagerBase : public cSimpleModule
{
    protected:
        typedef struct
        {
            bool hasArrivedAll;
            bool discardedAtRlc;
            bool discardedAtMac;
            bool sentOverTheAir;
            unsigned int pdcpSduSize;
            simtime_t entryTime;
        } PdcpStatus;

        int pktDiscardCounterTotal_; // total discarded packets counter of the node

        LteNodeType nodeType_; // UE or ENODED (used for set MACROS)
        short int harqProcesses_; // number of harq processes

        // when a mode switch occurs, the PDCP and RLC entities should be informed about the next SN to use,
        // i.e., the first SN not transmitted due to the mode switch
        unsigned int nextPdcpSno_;
        unsigned int nextRlcSno_;

        int headerCompressedSize_;

        virtual void initPdcpStatus(StatusDescriptor* desc, unsigned int pdcp, unsigned int headerSize, simtime_t& arrivalTime);
        virtual void removePdcpBurst(StatusDescriptor* desc, PdcpStatus& pdcpStatus,  unsigned int pdcpSno, bool ack);

        virtual int numInitStages() const { return 2; }
        virtual void initialize(int stage);

//        virtual bool hasFragments(LogicalCid lcid, unsigned int pdcp);

    public:
    
        PacketFlowManagerBase();

        // return true if a structure for this lcid is present
        virtual bool checkLcid(LogicalCid lcid) = 0;
        /*
         * initialize a new structure for this lcid
         * abstract since in EnodeB case, it initializes
         * different structure wrt the UE
         */
        virtual void initLcid(LogicalCid lcid, MacNodeId nodeId) = 0;

        /* reset the structure for this lcid
         * abstract since in EnodeB case, it clears
         * different structure wrt the UE
         */
        virtual void clearLcid(LogicalCid lcid) = 0;

        // reset structures for all connections
        virtual void clearAllLcid() = 0;


        /* 
        * This method insert a new pdcp seqnum and the corresponding entry time
        * @param lcid 
        * @param pdcpSno sequence number of the pdcp pdu
        * @param entryTime the time the packet enters PDCP layer
        */
        virtual void insertPdcpSdu(LogicalCid lcid, unsigned int pdcpSno, unsigned int sduSize, simtime_t entryTime) = 0;
        
        /* 
        * This method insert a new rlc seqnum and the corresponding pdcp pdus inside it
        * @param lcid 
        * @param rlcSno sequence number of the rlc pdu
        * @param pdcpSnoSet list of pdcp pdu inside the rlc pdu
        * @param lastIsFrag used to inform if the last pdcp is fragmented or not
        */
        virtual void insertRlcPdu(LogicalCid lcid, unsigned int rlcSno, SequenceNumberSet& pdcpSnoSet, bool lastIsFrag) = 0;
        
        /*
        * This method insert a new rlc seqnum and the corresponding pdcp pdus inside it
        * @param lcid
        * @param rlcPdu packet pointer
        */
        virtual void insertRlcPdu(LogicalCid lcid, LteRlcUmDataPdu* rlcPdu, RlcBurstStatus status) = 0;

        /* 
        * This method insert a new macPduId Omnet id and the corresponding rlc pdus inside it
        * @param lcid 
        * @param macPduId Omnet id of the mac pdu
        * @param rlcSnoSet list of rlc pdu inside the rlc pdu
        */  
        virtual void insertMacPdu(LogicalCid lcid, unsigned int macPduId, SequenceNumberSet& rlcSnoSet) = 0;

        /*
        * This method checks if the HARQ acSequenceNumberSetk relative to a macPduId acknowledges an ENTIRE
        * pdcp sdu 
        * @param lcid
        * @param macPduId Omnet id of the mac pdu 
        */
        virtual void macPduArrived(LogicalCid lcid, unsigned int macPduId) = 0;

        /*
        * This method is called after maxHarqTrasmission of a MAC PDU ID has been
        * reached. The PDCP, RLC, sno referred to the macPdu are cleared from the
        * data structures
        * @param lcid
        * @param macPduId Omnet id of the mac pdu to be discarded
        */
        virtual void discardMacPdu(LogicalCid lcid, unsigned int macPduId) = 0;

        /*
        * This method is used to take trace of all discarded RLC pdus. If all rlc pdus
        * that compose a PCDP SDU have been discarded the discarded counters are updated
        * @param lcid
        * @param rlcSno sequence number of the rlc pdu
        * @param fromMac used when this method is called by discardMacPdu
        */
        virtual void discardRlcPdu(LogicalCid lcid, unsigned int rlcSno, bool fromMac = false) = 0;

        virtual void insertHarqProcess(LogicalCid lcid, unsigned int harqProcId, unsigned int macPduId) = 0;

        /*
        * invoked by the MAC layer to notify that harqProcId is completed.
        * This method need to go back up the chain of sequence numbers to identify which
        * PDCP SDUs have been transmitted in this process.
        */
        // void notifyHarqProcess(LogicalCid lcid, unsigned int harqProcId) = 0;

        virtual void resetDiscardCounter();

        virtual unsigned int getNextRlcSno() { return nextRlcSno_; }
        virtual unsigned int getNextPdcpSno() { return nextPdcpSno_; }


        virtual ~PacketFlowManagerBase();
        virtual void finish();

};
#endif
