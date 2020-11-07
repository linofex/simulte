//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _L2MEASBASE_H_
#define _L2MEASBASE_H_

#include <vector>
#include <string>
#include <omnetpp.h>

class L2MeasBase
{
    private:
        std::string name_;
        std::vector<int> values_;
        int sum_;
        int mean_;
        int index_;
        int period_;
        int size_;
        bool movingAverage_;

        ::omnetpp::cOutVector outVector_;
        ::omnetpp::cHistogram histogram_;

    public:
        L2MeasBase();
        virtual void init(std::string name, int period, bool movingAverage);
        virtual ~L2MeasBase();
        virtual void addValue(int value);
        virtual int computeMean();
        virtual int getMean();
};


#endif //_L2MEASBASE_H_
