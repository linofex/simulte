//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//


#include "ENodeBCollector.h"
#include "corenetwork/statsCollector/StatsCollector.h"

using namespace std;

ENodeBCollector::ENodeBCollector(){}

void ENodeBCollector::initializeParameters(const StatsCollector* statsCollector){
    //get parent module for parameters, passed by constructor

    ttiPeriodPRBUsage_ = statsCollector->par("ttiPeriodPRBUsage");
    movingAverage_ = statsCollector->par("movingAverage");
    dl_total_prb_usage_cell.init(ttiPeriodPRBUsage_, movingAverage_);
    ul_total_prb_usage_cell.init(ttiPeriodPRBUsage_, movingAverage_);
    number_of_active_ue_dl_nongbr_cell.init(statsCollector->par("numberOfSamples"), false);
    number_of_active_ue_ul_nongbr_cell.init(statsCollector->par("numberOfSamples"), false);
};

void ENodeBCollector::add_number_of_active_ue_dl_nongbr_cell(int ues){
    number_of_active_ue_dl_nongbr_cell.addValue(ues);
}

void ENodeBCollector::add_number_of_active_ue_ul_nongbr_cell(int ues){
    number_of_active_ue_ul_nongbr_cell.addValue(ues);
}

void ENodeBCollector::add_dl_total_prb_usage_cell(double val){
    dl_total_prb_usage_cell.addValue(val);
}

void ENodeBCollector::add_ul_total_prb_usage_cell(double val){
    ul_total_prb_usage_cell.addValue(val);
}

int ENodeBCollector::get_dl_total_prb_usage_cell() {
    return dl_total_prb_usage_cell.getMean();
}

int ENodeBCollector::get_ul_total_prb_usage_cell() {
    return ul_total_prb_usage_cell.getMean();
}

int ENodeBCollector::get_number_of_active_ue_dl_nongbr_cell(){
    return number_of_active_ue_dl_nongbr_cell.getMean();
}

int ENodeBCollector::get_number_of_active_ue_ul_nongbr_cell(){
    return number_of_active_ue_ul_nongbr_cell.getMean();
}
