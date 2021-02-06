// @author Alessandro Noferi

package lte.corenetwork.nodes.mec.MEPlatform.MeServices.MeService_performance;

import lte.corenetwork.nodes.mec.MEPlatform.MeServices.MeServiceBase.MeServiceBase;

import inet.applications.contract.ITCPApp;


//
// Radio Network Information Service
// TCPApp that implements an HTTP server that responds to HTTP requests (e.g GET POST)
// from MeApplication, or any application that uses TCP connections.
// It stores all the connections in a map and gets the information for the responds by
// calling methods of EnodeB. (cross-call methods)
// 
// @see ~GenericAppMsg, ~TCPBasicClientApp, ~TelnetApp
//
simple LocationService_test like MeServiceBase, ITCPApp
{
    parameters:
        string localAddress = default(""); // local address; may be left empty ("")
        int localPort = default(1000);     // localPort number to listen on
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytestream"); // bytestreamfor emulation
        double LocationSubscriptionPeriod @unit(s)= default(1s);
        double requestServiceTime = default(5);
        double subscriptionServiceTime @unit(s) = default(0.5us);
        string serverThreadClass = default("SocketManager")
        @display("i=block/app");
        
		@signal[requestQueueSize](type = long);
        @statistic[requestQueueSizeStat](title="Request queue size"; source="requestQueueSize"; record=mean,vector);     
        
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

