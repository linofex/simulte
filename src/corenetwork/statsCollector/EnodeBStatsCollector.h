
//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_ENOBSTATSCOLLECTOR_H_
#define _LTE_ENOBSTATSCOLLECTOR_H_

#include <omnetpp.h>
#include "common/LteCommon.h"
#include "corenetwork/statsCollector/L2Measures/L2Measures.h"

using namespace inet;


class EnodeBStatsCollector: public cSimpleModule
{
    private:
        unsigned int ttiPeriodPRBUsage_;
        bool movingAverage_;
        PRBusage dl_total_prb_usage_cell;
        PRBusage ul_total_prb_usage_cell;
        ActiveUeSet number_of_active_ue_dl_nongbr_cell;
        ActiveUeSet number_of_active_ue_ul_nongbr_cell;


    public:
        EnodeBStatsCollector(){}
        virtual ~EnodeBStatsCollector(){}
        void add_dl_total_prb_usage_cell(double val);
        void add_ul_total_prb_usage_cell(double val);
        void add_number_of_active_ue_dl_nongbr_cell(int ues);
        void add_number_of_active_ue_ul_nongbr_cell(int ues);

        int get_dl_total_prb_usage_cell();
        int get_ul_total_prb_usage_cell();
        int get_number_of_active_ue_dl_nongbr_cell();
        int get_number_of_active_ue_ul_nongbr_cell();

        int get_dl_gbr_prb_usage_cell() {return -1;}
        int get_ul_gbr_prb_usage_cell() {return -1;}
        int get_dl_nongbr_prb_usage_cell() {return -1;}
        int get_ul_nongbr_prb_usage_cell() {return -1;}
        int get_received_dedicated_preambles_cell() {return -1;}
        int get_received_randomly_selected_preambles_low_range_cell() {return -1;}
        int get_received_randomly_selected_preambles_high_range_cell() {return -1;}
        int get_number_of_active_ue_dl_gbr_cell() {return -1;}
        int get_number_of_active_ue_ul_gbr_cell() {return -1;}

        int get_dl_gbr_pdr_cell() {return -1;}
        int get_ul_gbr_pdr_cell() {return -1;}
        int get_dl_nongbr_pdr_cell() {return -1;}
        int get_ul_nongbr_pdr_cell() {return -1;}


    protected:
        virtual void initialize(int stages);

        virtual int numInitStages() const { return INITSTAGE_LAST; }

        virtual void handleMessage(cMessage *msg)
        {
        }

};





#endif //_LTE_ENOBSTATSCOLLECTOR_H_
