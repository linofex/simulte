//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

//
//	@author	Angelo Buono
//

package lte.corenetwork.nodes.mec.MEPlatform;

import inet.linklayer.ppp.PPPInterface;
import inet.linklayer.loopback.LoopbackInterface;
import inet.linklayer.contract.IExternalNic;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.applications.contract.IUDPApp;
import inet.applications.contract.ITCPApp;
import inet.transportlayer.udp.UDP;
import inet.transportlayer.tcp.TCP;
import lte.corenetwork.nodes.mec.MEPlatform.IMEService;

import lte.corenetwork.nodes.mec.MEPlatform.GeneralServices.RadioNetworkInformation;



// 
//# VirtualisationInfrastructure
//

module MEPlatform2
{
    parameters:
        @networkNode();

        //# Network Layer specs
        string networkLayerType = default("IPv4NetworkLayer");
        string routingTableType = default("IPv4RoutingTable");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(routingTableType != "" ? absPath(".routingTable") : "");
        //# Num Services
        int numServices = default(0); // no of UDP ME Services. Specify the service types in INI file with udpManager.udpService[0..1].typename="UDPServ" syntax

        //# 
        int numTcpApps = default(0);

        @display("bgb=577,611");
    gates:

        inout ppp[];				// connection with GtpEndpoint and local/externa networks
        output meAppOut[];			// connection to the ME App input gate
        input meAppIn[];			// connection to the ME App output gate
        
        output meAppTcpOut[];			// connection to the ME App input gate
        input meAppTcpIn[];			// connection to the ME App output gate
        
        

    submodules:

        interfaceTable: InterfaceTable {
            @display("p=61.344,74.976;is=s");
        }

        // TODO: should not include a routing table
        // routing table
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=61.344,124.96;is=s");
        }

        //#
        //# ppp modules
        //#
        pppIf[sizeof(ppp)]: PPPInterface {
            @display("p=252,473");
        }

         lo0: LoopbackInterface {
                    @display("p=78,406");
                }
	     ext0: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=149,406,row,10;q=txQueue;i=block/ifcard");
        }

        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=147,54,row,60");
        }

        tcp: TCP {
            @display("p=200,231");
        }

        udp: UDP {
            @display("p=300,231");
        }

        udpService[numServices]: <> like IMEService {
            @display("p=237.25,81.9,row");
        }

        rniService: RadioNetworkInformation {
            @display("p=34,34,row");
        }

         //#
        //# Network layer module
        //#
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=252,343;q=queue");
        }


    connections allowunconnected:

		//# connect TCPapps to trasnsport layer
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }
        
        for i=0..numServices-1 {
            udpService[i].tcpOut --> tcp.appIn++;
            udpService[i].tcpIn <-- tcp.appOut++;
        }
        

        //#
        //# PPP interface to network layer connections
        //# 
        for i=0..sizeof(ppp)-1 {
            pppIf[i].phys <--> ppp++;
            pppIf[i].upperLayerOut --> networkLayer.ifIn++;
            pppIf[i].upperLayerIn <-- networkLayer.ifOut++;
        }
        
        
         networkLayer.ifOut++ --> lo0.upperLayerIn;
        lo0.upperLayerOut --> networkLayer.ifIn++;

        //# 
        //# Transport layer to network layer connections
        //#
        udp.ipOut --> networkLayer.transportIn++;
        udp.ipIn <-- networkLayer.transportOut++;

        tcp.ipOut --> networkLayer.transportIn++;
        tcp.ipIn <-- networkLayer.transportOut++;


//        for i=0..numExtInterfaces-1 {
//            ext[i].upperLayerOut --> networkLayer.ifIn++;
//            ext[i].upperLayerIn <-- networkLayer.ifOut++;
//        }
	        ext0.upperLayerOut --> networkLayer.ifIn++;
            ext0.upperLayerIn <-- networkLayer.ifOut++;
        //# 
        //# Internal connections: meAppIn & meAppOut gates to virtualisationManager.meAppIn & virtualisationManager.meAppOut
        //#
        //	IN VirtualisationManager initialize()
}
