//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


// @author Alessandro Noferi

package lte.apps.mec.MeServices.RNIService;


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
simple RNIService like ITCPApp
{
    parameters:
        string localAddress = default(""); // local address; may be left empty ("")
        int localPort = default(1000);     // localPort number to listen on
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytestream"); // bytestreamfor emulation
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

