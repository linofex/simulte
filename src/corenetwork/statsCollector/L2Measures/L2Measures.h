#ifndef _L2_MEASURES_H
#define  _L2_MEASURES_H

#include <math.h>       /* floor */
#include <vector>
#include <omnetpp.h>

struct PRBusage{
    double sum;
    int mean;
    std::vector<double> values;
    int index, period, size;
    bool movingAverage;
    ::omnetpp::cOutVector outVector;
    ::omnetpp::cHistogram histogram;

    void record() {
        histogram.recordAs("PRBusage");
    }
    void init(int length, bool moving){
        values = std::vector<double>(length);
        period = length;
        size = 0;
        index = 0;
        mean = 0;
        sum = 0;
        movingAverage = moving;
        histogram.setName("PRBusage");
        outVector.setName("PRBusage");
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
            mean = computeMean();
            outVector.record(mean);
            histogram.collect(mean);
        }
        else{
            if(index == period){ // compute mean at the end of the period
                mean = computeMean();
                outVector.record(mean);
                histogram.collect(mean);
            }
        }
    }

    int computeMean(){
        if(index == 0)
            return 0;

        if(!movingAverage && size < period) // no enough data
            return 0;
        else{
            int mean = floor(sum * 100/size);
            return mean < 0 ? 0: mean; // round could returns -0.00 -> -1
        }
    }

    int getMean(){
        return mean;
    }

};

struct ActiveUeSet {
    int sum, mean;
    std::vector<int> values;
    int index, period, size;
    bool movingAverage;
    ::omnetpp::cOutVector outVector;
    ::omnetpp::cHistogram histogram;


    void record() {
        histogram.recordAs("ActiveUeSet");
    }
    void init(int length, bool moving){
        values = std::vector<int>(length);
        period = length;
        size = 0;
        index = 0;
        sum = 0;
        mean = 0,
        movingAverage = moving;
        histogram.setName("ActiveUeSet");
        outVector.setName("ActiveUeSet");
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
            mean = computeMean();
            outVector.record(mean);
            histogram.collect(mean);
        }
        else{
            if(index == period){ // compute mean at the end of the period
                mean = computeMean();
                outVector.record(mean);
                histogram.collect(mean);
            }
        }
    }
    int getMean(){
        return mean;
    }
    int computeMean(){
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

struct DataVolume {
    int sum, mean, bytes;
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
        mean = 0;
        bytes = 0;
        movingAverage = moving;
        histogram.setName("DataVolume");
        outVector.setName("DataVolume");
    }
    void addBytes(int val)
    {
        bytes += val;
    }

    void saveBytesPeriod(){
        sum += bytes;

        if(size < period)
        {
            size++;
        }
        else
        {
            index = index%period;
            sum -= values[index];
        }
        values[index++] = bytes;
        bytes = 0;
        if(movingAverage){ // compute mean every new value
            mean = computeMean();
            outVector.record(mean);
            histogram.collect(mean);
        }
        else{
            if(index == period){ // compute mean at the end of the period
                mean = computeMean();
                outVector.record(mean);
                histogram.collect(mean);
            }
        }
    }

    void record() {
        histogram.recordAs("DataVolume");
    }


    int getMean(){
        return mean;
    }
    int computeMean(){
        // TODO convert to kBit
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
