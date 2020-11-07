
#ifndef _LTE_UECOLLECTOR_H_
#define _LTE_UECOLLECTOR_H_

#include <omnetpp.h>
#include "corenetwork/statsCollector/L2Measures/L2Measures.h"
//#include "corenetwork/statsCollector/StatsCollector.h"

class StatsCollector;

class UeCollector {
    private:
        unsigned int ttiPeriodPRBUsage_;
        bool movingAverage_;


    public:
        UeCollector();
        UeCollector(const StatsCollector* statsCollector);
        virtual ~UeCollector(){}
};


#endif // LTE_UECOLLECTOR_H_
