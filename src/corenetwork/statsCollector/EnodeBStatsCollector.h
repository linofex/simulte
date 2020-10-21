
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

struct L2Meas{
    double sum;
    double* values;
    int index, period, size;
    cOutVector outVector;
    cHistogram histogram;
    simsignal_t signal;


    void registerSignale(simsignal_t signal_ ){
        signal = signal_;
    }

    void initVector(int length){
        values = new double[length];
        period = length;
        size = 0;
        index = 0;
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
        double mean = getMean();
        outVector.record(mean);
        histogram.collect(mean);
    }

    double getMean(){
        if(index == 0)
            return 0;
            //eccezione;
        else{
            return sum/size;
        }

    }

};


class EnodeBStatsCollector: public cSimpleModule
{
    private:
        unsigned int ttiPeriodPRBUsage_;
        struct L2Meas dl_total_prb_usage_cell;
        struct L2Meas ul_total_prb_usage_cell;

    public:
        EnodeBStatsCollector(){
            dl_total_prb_usage_cell.values = nullptr;
            ul_total_prb_usage_cell.values = nullptr;

        }
        virtual ~EnodeBStatsCollector(){
            dl_total_prb_usage_cell.del();
            ul_total_prb_usage_cell.del();

        }
        void add_dl_total_prb_usage_cell(double val);
        void add_ul_total_prb_usage_cell(double val);
        double get_dl_total_prb_usage_cell();
        double get_ul_total_prb_usage_cell();


    protected:
        virtual void initialize(int stages);

        virtual int numInitStages() const { return INITSTAGE_LAST; }

        virtual void handleMessage(cMessage *msg)
        {
        }

};





#endif //_LTE_ENOBSTATSCOLLECTOR_H_
