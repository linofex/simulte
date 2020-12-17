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


package lte.apps.mec.PerformanceEvaluation.MeApp.UeApp;

import inet.applications.contract.ITCPApp;

//
simple UeApp like ITCPApp
{
    parameters:
        string localAddress = default(""); // local address; may be left empty ("")
        int localPort; // port number to listen on
        double echoFactor = default(1);
        double echoDelay @unit(s) = default(0s);
        string dataTransferMode @enum("bytecount","object","bytestream") = default("object");
        @display("i=block/app");
       
       // signals
       	@signal[propagationTime](type = simtime_t);
		@statistic[propagationTimeStat](source = "propagationTime"; record =vector, mean;);
		
		@signal[processingTime](type = simtime_t);
		@statistic[processingTimeStat](source = "processingTime"; record  =vector, mean;);
		
		
		@signal[deltaProcessing](type = long);
		@statistic[deltaProcessingStat](source = "deltaProcessing"; record  =vector, mean;);
		
		@signal[deltaPropagation](type = long);
		@statistic[deltaPropagationStat](source = "deltaPropagation"; record  =vector, mean;);
		
		
	//		@statistic[waitingTimeHealingQueueStat](source="waitingTimeHealingQueue"; record=vector,mean;);
       
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

