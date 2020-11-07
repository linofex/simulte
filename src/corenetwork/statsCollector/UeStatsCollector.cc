//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "corenetwork/statsCollector/UeStatsCollector.h"


using namespace std;

Define_Module(UeStatsCollector);


void UeStatsCollector::initialize(int stage){
    if (stage == inet::INITSTAGE_LOCAL)
       {
        ul_non_gbr_pdr_ue.init("dl_total_prb_usage_cell", par("ttiPeriodPRBUsage"), par("movingAverage"));
        ul_non_gbr_data_volume_ue.init("ul_total_prb_usage_cell", par("ttiPeriodPRBUsage"), par("movingAverage"));
       }
}


void UeStatsCollector::add_ul_non_gbr_pdr_ue(int val){
    ul_non_gbr_pdr_ue.addValue(val);
}

void UeStatsCollector::add_ul_non_gbr_data_volume_ue(int val){
    ul_non_gbr_data_volume_ue.addValue(val);
}

int UeStatsCollector::get_ul_non_gbr_data_volume_ue() {
    return ul_non_gbr_data_volume_ue.getMean();
}

int UeStatsCollector::get_ul_non_gbr_pdr_ue() {
    return ul_non_gbr_pdr_ue.getMean();
}

