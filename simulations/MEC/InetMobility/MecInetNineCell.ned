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

package lte.simulations.MEC.InetMobility;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;

import lte.world.radio.LteChannelControl;
import lte.epc.PgwStandardSimplified;
import lte.corenetwork.binder.LteBinder;
import lte.corenetwork.nodes.eNodeB;
import lte.corenetwork.nodes.Ue;
import lte.corenetwork.nodes.carss.Car;

//mec
import lte.corenetwork.nodes.mec.MEHost;
import lte.corenetwork.nodes.mec.VirtualisationInfrastructure.VirtualisationInfrastructure;
import lte.corenetwork.nodes.mec.MEPlatform.MEPlatform;

//veins mobility
//import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;

//statistics
import lte.apps.d2dMultihop.statistics.MultihopD2DStatistics;

network MecInetNineCell
{
    parameters:
        @display("bgb=1000, 4000");

        int numCars = default(1);
        int numClusters = default(1);
        int numRoads = default(1);

        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)

    submodules:

        routingRecorder: RoutingTableRecorder {
            @display("p=240.32,781.04004;is=s");
        }

        configurator: IPv4NetworkConfigurator {
            @display("p=246.328,240.32");
            config = xmldoc("demo.xml");
        }

        d2dMultihopStatistics: MultihopD2DStatistics {
            @display("p=240.32,1003.33606;is=s");
        }

        //# LTE modules
        channelControl: LteChannelControl {
            @display("p=240.32,696.92804;is=s");
        }

        binder: LteBinder {
            @display("p=246.328,318.424;is=s");
        }

        eNodeB1: eNodeB {	@display("p=100,150;is=vl");}

        eNodeB2: eNodeB {	@display("p=200,450;is=vl");}

        eNodeB3: eNodeB {	@display("p=300,750;is=vl");}

        eNodeB4: eNodeB {	@display("p=300,1050;is=vl");}
            
        eNodeB5: eNodeB {	@display("p=200,1350;is=vl");}

        eNodeB6: eNodeB {	@display("p=100,1750;is=vl");}
        
        eNodeB7: eNodeB {	@display("p=100,2050;is=vl");}
        
		eNodeB8: eNodeB {	@display("p=200,2350;is=vl");}
		
		eNodeB9: eNodeB {	@display("p=300,2650;is=vl");}

        
        pgw: PgwStandardSimplified {
            @display("p=1958.608,1520.024;is=l");
            nodeType = "PGW";
        }

        car[numCars]: Car {
            @display("p=444.592,696.92804");
        }

        //# MEC module
        meHost: MEHost {
            @display("p=1910.5441,606.80804;is=l");
        }

    connections allowunconnected:

        pgw.pppg++ <--> Eth10G <--> eNodeB1.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB2.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB3.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB4.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB5.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB6.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB7.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB8.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB9.ppp;


        eNodeB1.x2++ <--> Eth10G <--> eNodeB2.x2++;
        eNodeB2.x2++ <--> Eth10G <--> eNodeB3.x2++;
        eNodeB3.x2++ <--> Eth10G <--> eNodeB4.x2++;
        eNodeB4.x2++ <--> Eth10G <--> eNodeB5.x2++;
        eNodeB5.x2++ <--> Eth10G <--> eNodeB6.x2++;
        eNodeB6.x2++ <--> Eth10G <--> eNodeB7.x2++;
        eNodeB7.x2++ <--> Eth10G <--> eNodeB8.x2++;
        eNodeB8.x2++ <--> Eth10G <--> eNodeB9.x2++;
        
              



        //#	mec
        // direct connection between eNodeB2 and meHost
        meHost.pppENB++ <--> Eth10G <--> eNodeB1.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB2.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB3.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB4.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB5.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB6.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB7.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB8.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB9.pppMEHost;
        

//        eNodeB4.x2++ <--> Eth10G <--> eNodeB5.x2++;
}
