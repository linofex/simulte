

//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//
#ifndef _LTE_UESTATSCOLLECTOR_H_
#define _LTE_UESTATSCOLLECTOR_H_

#include <omnetpp.h>
#include "common/LteCommon.h"
#include "common/MecCommon.h"
#include "corenetwork/statsCollector/L2Measures/L2MeasBase.h"
#include <string>
#include "corenetwork/statsCollector/UeStatsCollector.h"



using namespace inet;

/**
 *
 * TODO
 */

class LtePdcpRrcUe;
class LteMacUe;
class PacketFlowManagerUe;
class UeStatsCollector: public cSimpleModule
{
    private:




        // Used by the RNI service
        mec::AssociateId associateId_;

        // LTE Nic layers
        LtePdcpRrcUe *pdcp_;
        LteMacUe     *mac_;
        PacketFlowManagerUe *flowManager_;

        // packet delay
        L2MeasBase ul_nongbr_delay_ue;
        L2MeasBase dl_nongbr_delay_ue;
        // packet discard rate
        L2MeasBase ul_nongbr_pdr_ue;
        L2MeasBase dl_nongbr_pdr_ue;
        // scheduled throughput
        L2MeasBase ul_nongbr_throughput_ue;
        L2MeasBase dl_nongbr_throughput_ue;
        // data volume
        L2MeasBase ul_nongbr_data_volume_ue;
        L2MeasBase dl_nongbr_data_volume_ue;

        // inserire segnali

        bool handover_;

    public:
        UeStatsCollector(){}
        virtual ~UeStatsCollector(){}

        void add_ul_nongbr_delay_ue();
        void add_dl_nongbr_delay_ue(double value);// called by the eNodeBCollector

        void add_ul_nongbr_pdr_ue(double value); // called by this module, but from a method
        void add_dl_nongbr_pdr_ue(double value); // called by the eNodeBCollector
        DiscardedPkts getULDiscardedPkt();

        void add_ul_nongbr_throughput_ue(double value);
        void add_dl_nongbr_throughput_ue(double value);

        void add_ul_nongbr_data_volume_ue(unsigned int value); // called by the eNodeBCollector
        void add_dl_nongbr_data_volume_ue(unsigned int value); // called by the eNodeBCollector


        int get_ul_nongbr_delay_ue();
        int get_dl_nongbr_delay_ue();

        int get_ul_nongbr_pdr_ue();
        int get_dl_nongbr_pdr_ue();

        int get_ul_nongbr_throughput_ue();
        int get_dl_nongbr_throughput_ue();

        int get_ul_nongbr_data_volume_ue();
        int get_dl_nongbr_data_volume_ue();

        int get_dl_gbr_delay_ue(){return -1;}
        int get_ul_gbr_delay_ue(){return -1;}
        int get_dl_gbr_pdr_ue(){return -1;}
        int get_ul_gbr_pdr_ue(){return -1;}
        int get_dl_gbr_throughput_ue(){return -1;}
        int get_ul_gbr_throughput_ue(){return -1;}
        int get_dl_gbr_data_volume_ue(){return -1;}
        int get_ul_gbr_data_volume_ue(){return -1;}


        void sedHandover(bool value)
        {
            handover_ = value;
        }

        mec::AssociateId getAssociateId() const
        {
            return associateId_;
        }


    protected:
        virtual void initialize(int stages);

        virtual int numInitStages() const { return INITSTAGE_LAST; }

        virtual void handleMessage(cMessage *msg)
        {
        }

};





#endif //_LTE_ENOBSTATSCOLLECTOR_H_
