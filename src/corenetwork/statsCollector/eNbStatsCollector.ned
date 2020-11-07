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
        
        // time periods for measurements 
        int ttiPeriodPRBUsage = default(5);
         
        
        @display("i=block/cogwheel");
}