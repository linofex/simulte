
//


// @author Alessandro Noferi

package lte.corenetwork.nodes.mec.MEPlatform.MeServices.MeServiceBase;


moduleinterface MeServiceBase 
{
    parameters:
        @display("i=block/app");
//        @signal[sentPk](type=inet::GenericAppMsg);
//        @signal[rcvdPk](type=inet::GenericAppMsg);
//        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
//        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
//        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

