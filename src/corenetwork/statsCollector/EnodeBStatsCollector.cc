//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "corenetwork/statsCollector/EnodeBStatsCollector.h"

Define_Module(EnodeBStatsCollector);


void EnodeBStatsCollector::initialize(int stage){
    if (stage == inet::INITSTAGE_LOCAL)
       {
        dl_total_prb_usage_cell.init("dl_total_prb_usage_cell", par("ttiPeriodPRBUsage"), par("movingAverage"));
        ul_total_prb_usage_cell.init("ul_total_prb_usage_cell", par("ttiPeriodPRBUsage"), par("movingAverage"));
        number_of_active_ue_dl_nongbr_cell.init("number_of_active_ue_dl_nongbr_cell", par("numberOfSamples"), false);
        number_of_active_ue_ul_nongbr_cell.init("number_of_active_ue_ul_nongbr_cell", par("numberOfSamples"), false);
       }
}


void EnodeBStatsCollector::add_dl_total_prb_usage_cell(int value)
{
    dl_total_prb_usage_cell.addValue(value);
}

void EnodeBStatsCollector::add_ul_total_prb_usage_cell(int value){
    ul_total_prb_usage_cell.addValue(value);
}


void EnodeBStatsCollector::add_number_of_active_ue_dl_nongbr_cell(int value)
{
    number_of_active_ue_dl_nongbr_cell.addValue(value);
}

void EnodeBStatsCollector::add_number_of_active_ue_ul_nongbr_cell(int value)
{
    number_of_active_ue_ul_nongbr_cell.addValue(value);
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


