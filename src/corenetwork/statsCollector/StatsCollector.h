
//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_STATSCOLLECTOR_H_
#define _LTE_STATSCOLLECTOR_H_

#include <omnetpp.h>
#include <map>
#include "common/LteCommon.h"
#include "corenetwork/statsCollector/UeCollector.h"
#include "corenetwork/statsCollector/ENodeBCollector.h"




using namespace inet;

typedef std::map<MacNodeId, UeCollector*> UeCollectors;

class StatsCollector: public cSimpleModule
{

    private:
        UeCollectors ueCollectors_;
        ENodeBCollector eNBCollector_;

    public:
        StatsCollector();

        virtual ~StatsCollector();
    protected:
        virtual void initialize(int stages);

        virtual int numInitStages() const { return INITSTAGE_LAST; }

        virtual void handleMessage(cMessage *msg)
        {
        }
    public:
        ENodeBCollector* getENodeBCollector();
        UeCollectors* getUeCollectors();
        UeCollector* getUeCollector(MacNodeId id);
        void addUe(MacNodeId id);
        void attachUe(MacNodeId id, UeCollector* collector); // remember to change ECGI
        UeCollector* detachUe(MacNodeId id);
        void deleteUe(MacNodeId id);


};

#endif //_LTE_STATSCOLLECTOR_H_
