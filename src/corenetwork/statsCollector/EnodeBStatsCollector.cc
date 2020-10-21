//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "corenetwork/statsCollector/EnodeBStatsCollector.h"


using namespace std;

Define_Module(EnodeBStatsCollector);


void EnodeBStatsCollector::initialize(int stage){
    if (stage == inet::INITSTAGE_LOCAL)
       {
        ttiPeriodPRBUsage_ = par("ttiPeriodPRBUsage");
        dl_total_prb_usage_cell.initVector(ttiPeriodPRBUsage_);
        ul_total_prb_usage_cell.initVector(ttiPeriodPRBUsage_);

       }
}


void EnodeBStatsCollector::add_dl_total_prb_usage_cell(double val){
    dl_total_prb_usage_cell.addValue(val);
}

void EnodeBStatsCollector::add_ul_total_prb_usage_cell(double val){
    ul_total_prb_usage_cell.addValue(val);
}

double EnodeBStatsCollector::get_dl_total_prb_usage_cell() {
    return dl_total_prb_usage_cell.getMean();
}

double EnodeBStatsCollector::get_ul_total_prb_usage_cell() {
    return ul_total_prb_usage_cell.getMean();
}
