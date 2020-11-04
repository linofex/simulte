#ifndef _L2_MEASURES_H
#define  _L2_MEASURES_H

#include <math.h>       /* floor */
#include <vector>
#include <omnetpp.h>

struct PRBusage{
    double sum;
    std::vector<double> values;
    int index, period, size;
    bool movingAverage;
    ::omnetpp::cOutVector outVector;
    ::omnetpp::cHistogram histogram;

    void init(int length, bool moving){
        values = std::vector<double>(length);
        period = length;
        size = 0;
        index = 0;
        sum = 0;
        movingAverage = moving;
    }

    void addValue(double val){
        sum += val;
        if(size < period)
        {
            size++;
        }
        else
        {
            index = index%period;
            sum -= values[index];
        }
        values[index++] = val;

        if(movingAverage){ // compute mean every new value
            double mean = getMean();
            outVector.record(mean);
            histogram.collect(mean);
        }
        else{
            if(index == period){ // compute mean at the end of the period
                double mean = getMean();
                outVector.record(mean);
                histogram.collect(mean);
            }
        }
    }

    int getMean(){
        if(index == 0)
            return 0;

        if(!movingAverage && size < period) // no enough data
            return 0;
        else{
            int mean = floor(sum * 100/size);
            return mean < 0 ? 0: mean; // round could returns -0.00 -> -1
        }

    }

};



struct ActiveUeSet {
    int sum;
    std::vector<int> values;
    int index, period, size;
    bool movingAverage;
    ::omnetpp::cOutVector outVector;
    ::omnetpp::cHistogram histogram;


    void init(int length, bool moving){
        values = std::vector<int>(length);
        period = length;
        size = 0;
        index = 0;
        sum = 0;
        movingAverage = moving;
    }

    void addValue(int val){
        sum += val;
        if(size < period)
        {
            size++;
        }
        else
        {
            index = index%period;
            sum -= values[index];
        }
        values[index++] = val;

        if(movingAverage){ // compute mean every new value
            double mean = getMean();
            outVector.record(mean);
            histogram.collect(mean);
        }
        else{
            if(index == period){ // compute mean at the end of the period
                double mean = getMean();
                outVector.record(mean);
                histogram.collect(mean);
            }
        }
    }

    int getMean(){
        if(index == 0)
            return 0;

        if(!movingAverage && size < period) // no enough data
            return 0;
        else{
            int mean = floor(sum/size);
            return mean < 0 ? 0: mean; // round could returns -0.00 -> -1
        }

    }

};



#endif
