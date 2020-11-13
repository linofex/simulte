
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
#include <map>
#include "corenetwork/statsCollector/L2Measures/L2MeasBase.h"

using namespace inet;

/**
 *
 * TODO
 */

class UeStatsCollector;
class LteMacEnb;
class LtePdcpRrcEnb;
class PacketFlowManager;

class EnodeBStatsCollector: public cSimpleModule
{
    private:
        // LTE Nic layers
        LtePdcpRrcEnb *pdcp_;
        LteMacEnb     *mac_;
        PacketFlowManager *flowManager_;

        typedef std::map<MacNodeId, UeStatsCollector*> UeStatsCollectorMap;
        UeStatsCollectorMap ueCollectors_;

        // L2 Measures per EnodeB
        L2MeasBase dl_total_prb_usage_cell;
        L2MeasBase ul_total_prb_usage_cell;
        L2MeasBase number_of_active_ue_dl_nongbr_cell;
        L2MeasBase number_of_active_ue_ul_nongbr_cell;
        L2MeasBase dl_nongbr_pdr_cell;

        // inserire segnali?


        /*
         * timers:
         * - PRB usage (TTI)
         * - # active UEs
         * - Discard rate
         * - Packet delay
         */



    public:
        EnodeBStatsCollector(){}
        virtual ~EnodeBStatsCollector(){}

        // UeStatsCollector management methods

        /*
         * addUeCollector adds the UE collector to the list;
         * Called during UE initialization and after handover
         * where the UE moves to the eNobeB of this EnodeBStatsCollector
         * @param id of the UE to add
         * @param ueCollector pointer to the ueStatsCollector
         */
        void addUeCollector(MacNodeId id, UeStatsCollector* ueCollector);

        /*
         * removeUeCollector removes the UE collector from the list;
         * Called during UE termination and after handover
         * where the UE moves to another eNobeB
         * @param id of the UE to remove
         */
        void removeUeCollector(MacNodeId id);

        /*
         * getUeCollector returns:
         *  - the UeStatsCollector of an UE;
         *  - nullptr if nodeId is not present
         *
         * @param id relative to a UeStatsCollector
         */
        UeStatsCollector* getUeCollector(MacNodeId id);

        /*
         * hasUeCollector returns true if the EnodeB
         * associated to this eNodeBCollector
         *
         * @param id relative to a UeStatsCollector
         */
        bool hasUeCollector(MacNodeId id);

        // L2 measurements methods

        void add_dl_total_prb_usage_cell();
        void add_ul_total_prb_usage_cell();
        void add_number_of_active_ue_dl_nongbr_cell();
        void add_number_of_active_ue_ul_nongbr_cell();
        void add_dl_nongbr_pdr_cell();
        void add_ul_nongbr_pdr_cell();

        int get_dl_total_prb_usage_cell();
        int get_ul_total_prb_usage_cell();
        int get_dl_nongbr_prb_usage_cell();
        int get_ul_nongbr_prb_usage_cell();

        int get_number_of_active_ue_dl_nongbr_cell();
        int get_number_of_active_ue_ul_nongbr_cell();
        int get_dl_nongbr_pdr_cell();
        int get_ul_nongbr_pdr_cell();

        // save stats into UeCollectors
        void add_dl_nongbr_delay_perUser();
        void add_dl_nongbr_pdr_cell_perUser();
        void add_ul_nongbr_data_volume_ue_perUser();
        void add_dl_nongbr_data_volume_ue_perUser();

        int get_dl_gbr_prb_usage_cell(){return -1;}
        int get_ul_gbr_prb_usage_cell(){return -1;}
        int get_received_dedicated_preambles_cell(){return -1;}
        int get_received_randomly_selected_preambles_low_range_cell(){return -1;}
        int get_received_randomly_selected_preambles_high_range_cell(){return -1;}
        int get_number_of_active_ue_dl_gbr_cell(){return -1;}
        int get_number_of_active_ue_ul_gbr_cell(){return -1;}
        int get_dl_gbr_pdr_cell(){return -1;}
        int get_ul_gbr_pdr_cell(){return -1;}



    protected:
        virtual void initialize(int stages);

        virtual int numInitStages() const { return INITSTAGE_LAST; }

        virtual void handleMessage(cMessage *msg)
        {
        }

};





#endif //_LTE_ENOBSTATSCOLLECTOR_H_
