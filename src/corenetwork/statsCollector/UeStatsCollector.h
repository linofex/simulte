

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
#include "corenetwork/statsCollector/L2Measures/L2MeasBase.h"
#include <omnetpp.h>
#include <string>

#include "corenetwork/statsCollector/L2Measures/L2MeasBase.h"

using namespace inet;

/**
 *
 * TODO
 */



class UeStatsCollector: public cSimpleModule
{
    private:
        L2MeasBase ul_non_gbr_pdr_ue;
        L2MeasBase ul_non_gbr_data_volume_ue;


        // inserire segnali

    public:
        UeStatsCollector(){}
        virtual ~UeStatsCollector(){}
        void add_ul_non_gbr_pdr_ue(int val);
        int get_ul_non_gbr_pdr_ue();

        void add_ul_non_gbr_data_volume_ue(int val);
        int get_ul_non_gbr_data_volume_ue();



    protected:
        virtual void initialize(int stages);

        virtual int numInitStages() const { return INITSTAGE_LAST; }

        virtual void handleMessage(cMessage *msg)
        {
        }

};





#endif //_LTE_ENOBSTATSCOLLECTOR_H_
