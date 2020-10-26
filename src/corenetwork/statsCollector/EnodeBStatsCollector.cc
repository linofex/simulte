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
        movingAverage_ = par("movingAverage");
        dl_total_prb_usage_cell.init(ttiPeriodPRBUsage_, movingAverage_);
        ul_total_prb_usage_cell.init(ttiPeriodPRBUsage_, movingAverage_);
        number_of_active_ue_dl_nongbr_cell.init(par("numberOfSamples"), false);
        number_of_active_ue_ul_nongbr_cell.init(par("numberOfSamples"), false);
       }
}

void EnodeBStatsCollector::add_number_of_active_ue_dl_nongbr_cell(int ues){
    number_of_active_ue_dl_nongbr_cell.addValue(ues);
}

void EnodeBStatsCollector::add_number_of_active_ue_ul_nongbr_cell(int ues){
    number_of_active_ue_ul_nongbr_cell.addValue(ues);
}

void EnodeBStatsCollector::add_dl_total_prb_usage_cell(double val){
    dl_total_prb_usage_cell.addValue(val);
}

void EnodeBStatsCollector::add_ul_total_prb_usage_cell(double val){
    ul_total_prb_usage_cell.addValue(val);
}

int EnodeBStatsCollector::get_dl_total_prb_usage_cell() {
    return dl_total_prb_usage_cell.getMean();
}

int EnodeBStatsCollector::get_ul_total_prb_usage_cell() {
    return ul_total_prb_usage_cell.getMean();
}

int EnodeBStatsCollector::get_number_of_active_ue_dl_nongbr_cell(){
    return number_of_active_ue_dl_nongbr_cell.getMean();
}

int EnodeBStatsCollector::get_number_of_active_ue_ul_nongbr_cell(){
    return number_of_active_ue_ul_nongbr_cell.getMean();
}

