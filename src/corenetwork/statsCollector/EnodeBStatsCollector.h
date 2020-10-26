
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
#include <string>

#include "common/LteCommon.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/L3Address.h"
#include "corenetwork/binder/PhyPisaData.h"
#include "corenetwork/nodes/ExtCell.h"
#include "stack/mac/layer/LteMacBase.h"
#include <math.h>       /* floor */

using namespace inet;

/**
 * The LTE Binder module has one instance in the whole network.
 * It stores global mapping tables with OMNeT++ module IDs,
 * IP addresses, etc.
 *
 * After this it fills the two tables:
 * - nextHop, binding each master node id with its slave
 * - nodeId, binding each node id with the module id used by Omnet.
 * - dMap_, binding each master with all its slaves (used by amc)
 *
 * The binder is accessed to gather:
 * - the nextHop table (by the eNodeB)
 * - the Omnet module id (by any module)
 * - the map of deployed UEs per master (by amc)
 *
 */

struct L2MeasGeneric{
    double sum;
    double* values;
    int index, period, size;
    bool movingAverage;
    cOutVector outVector;
    cHistogram histogram;
    simsignal_t signal;


    void registerSignale(simsignal_t signal_ ){
        signal = signal_;
    }

    void init(int length, bool moving){
        values = new double[length];
        period = length;
        size = 0;
        index = 0;
        sum = 0;
        movingAverage = moving;
    }

    void del(){
        delete [] values;
    }

    void addValue(double val){
        sum += val;
        if(size < period)
            size++;
        else{
            index = index%period;
            sum -= values[index];
        }
        values[index++] = val;
        if(movingAverage){ // each TTI
            double mean = getMean();
            outVector.record(mean);
            histogram.collect(mean);
        }
        else{
            if(index == period){ // each sample*period
                double mean = getMean();
                outVector.record(mean);
                histogram.collect(mean);
            }
        }
    }

    int getMean(){
        if(index == 0)
            return 0;
            //eccezione;
        if(!movingAverage && size < period) // fixed average and no enough data
            return 0;
        else{
            int mean = (int)floor(sum/size);
            return mean < 0? 0: mean;
        }

    }

};


class EnodeBStatsCollector: public cSimpleModule
{
    private:
        unsigned int ttiPeriodPRBUsage_;
        bool movingAverage_;
        struct L2MeasGeneric dl_total_prb_usage_cell;
        struct L2MeasGeneric ul_total_prb_usage_cell;
        struct L2MeasGeneric number_of_active_ue_dl_nongbr_cell;
        struct L2MeasGeneric number_of_active_ue_ul_nongbr_cell;


    public:
        EnodeBStatsCollector(){
            dl_total_prb_usage_cell.values = nullptr;
            ul_total_prb_usage_cell.values = nullptr;
            number_of_active_ue_dl_nongbr_cell.values = nullptr;
            number_of_active_ue_dl_nongbr_cell.values = nullptr;

        }
        virtual ~EnodeBStatsCollector(){
            dl_total_prb_usage_cell.del();
            ul_total_prb_usage_cell.del();
            number_of_active_ue_dl_nongbr_cell.del();
            number_of_active_ue_ul_nongbr_cell.del();

        }
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
