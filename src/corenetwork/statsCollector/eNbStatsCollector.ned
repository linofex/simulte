// 
//                           SimuLTE
// 
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself, 
// and cannot be removed from it.
// 


package lte.corenetwork.statsCollector;
// 
// This is the statistic collector for an eNodeB. It stores all the attributes of the RNI 
// service Layer2Meaurements resource. The RNI service will call its methods in order to 
// responds to requests

//
simple EnodeBStatsCollector 
{
    parameters:

		bool movingAverage = default(false);
        
        // used to calculate l2meas as averages 
        int prbUsagePeriods = default(5);
        int dataVolumePeriods = default(2);
        int activeUserPeriods = default(10); //10*100ms = 1s
        int delayPacketPeriods = default(2); //used by the ue collector, remove 
        int discardRatePeriods = default(2);

		// period duration for each l2 meas
		double prbUsagePeriod  @unit(ms)   = default(1ms); // TTI period
        double activeUserPeriod @unit(ms)  = default(100ms);
        double dataVolumePeriod @unit(s)   = default(1s);
        double delayPacketPeriod @unit(s)  = default(0.5s);
        double discardRatePeriod @unit(s) = default(60s);

        @display("i=block/cogwheel");
}