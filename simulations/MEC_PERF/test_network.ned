//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package lte.simulations.MEC_PERF;


import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import lte.corenetwork.binder.LteBinder;
import lte.corenetwork.nodes.Ue;
import lte.corenetwork.nodes.eNodeB;
import lte.corenetwork.nodes.ExtCell;
import lte.epc.PgwStandardSimplified;
import lte.world.radio.LteChannelControl;
//mec
import lte.corenetwork.nodes.mec.MEHost;
import lte.corenetwork.nodes.mec.VirtualisationInfrastructure.VirtualisationInfrastructure;
import lte.corenetwork.nodes.mec.MEPlatform.MEPlatform;

network test_network
{
    parameters:
        int numExtCells = default(0);
        @display("i=block/network2;bgb=991,558;bgi=background/budapest");
    submodules:
        channelControl: LteChannelControl {
            @display("p=50,25;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=50,75;is=s");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=50,125");
            config = xmldoc("demo.xml");
        }
        binder: LteBinder {
            @display("p=50,175;is=s");
        }
        server: StandardHost {
            @display("p=169.15,83.33125;is=n;i=device/server");
        }
        pgw: PgwStandardSimplified {
            nodeType = "PGW";
            @display("p=389.29376,57.212498;is=l");
        }
        router: Router {
            @display("p=274.86874,82.0875;i=device/smallrouter");
        }
        eNodeB1: eNodeB {
            @display("p=310.9375,182.83125;is=vl");
        }
        eNodeB2: eNodeB {
            @display("p=522.375,160.44376;is=vl");
        }
        ue11: Ue {
            @display("p=227.60625,279.84375");
        }
        ue12: Ue {
            @display("p=322.13126,349.49374");
        }
        ue21: Ue {
            @display("p=575.85626,292.28125");
        }
        ue22: Ue {
            @display("p=701.475,195.26875");
        }
//        extCell[numExtCells]: ExtCell {
//            @display("p=100,480;is=vl");
//        }


        //# MEC module
        meHost: MEHost {
            @display("p=425.3625,258.7;is=l");
        }

    connections:
        server.pppg++ <--> Eth10G <--> router.pppg++;
        router.pppg++ <--> Eth10G <--> pgw.filterGate;
        pgw.pppg++ <--> Eth10G <--> eNodeB1.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB2.ppp;

        //# X2 connections
        eNodeB1.x2++ <--> Eth10G <--> eNodeB2.x2++;

        //#	mec
        // direct connection between eNodeB2 and meHost
        meHost.pppENB++ <--> Eth10G <--> eNodeB1.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB2.pppMEHost;

}
