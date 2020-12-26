//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

//
//  @author Angelo Buono
//

package lte.corenetwork.nodes.mec.MEPlatform.MEClusterizeService;

import lte.corenetwork.nodes.mec.MEPlatform.IMEService;

import inet.applications.contract.ITCPApp;


//
//#	MEClusterizeService
//
//	Simple Module for a ME Service communicating with a ME App on a ME HOST.
//	The task of this module is:
//								1)

simple MEClusterizeService like IMEService, ITCPApp
{
    parameters:
        //auto-scheduling informations
        double startTime @unit("s") = default(0s);
        double period @unit("s") = default(0.2s);
        double getPeriod @unit("s") = default(0.02s);
        string connectAddress = default("");  // server address (may be symbolic)
        int connectPort = default(1000); // port number to connect to
        
        bool speed = default(false);
        
        //txMode information: how to send the INFO_MEAPP ClusterizeConfigPacket to the UEClusterizeApp: from ClusterizePacket_types.h
        // V2V_UNICAST_TX_MODE :				"V2V_UNICAST"
		// V2V_MULTICAST_TX_MODE :				"V2V_MULTICAST"
		// DOWNLINK_UNICAST_TX_MODE : 			"DOWNLINK_UNICAST"
		// DOWNLINK_MULTICAST_TX_MODE :			"DOWNLINK_MULTICAST"			not supported by Lte Nic
		// HYBRID_TX_MODE:						"HYBRID"						not yet implemented in MEClusterizeService.cpp
       	string preconfiguredTxMode = default("DOWNLINK_UNICAST");
       	string dataTransferMode @enum("bytecount","object","bytestream") = default("bytestream");
       	string serverThreadClass = default("SocketManager");
       	
       	
       	
       	
    gates:
        
        input meAppIn[];
        output meAppOut[];     
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}