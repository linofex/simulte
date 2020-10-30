#ifndef _L2MEASBASE_H_
#define _L2MEASBASE_H_

#include <omnetpp.h>


template <typename T>
class L2MeasBase {
    private:
        EnodeBStatsCollector *collector; // used for par
        T sum;
        std::vector<T> values;
        int index, period, size;
        bool movingAverage;
        cOutVector outVector;
        cHistogram histogram;
        simsignal_t signal;

    public:
        L2MeasBase<T>(){
            collector = nullptr;
        }




};







#endif //_L2MEASBASE_H_
