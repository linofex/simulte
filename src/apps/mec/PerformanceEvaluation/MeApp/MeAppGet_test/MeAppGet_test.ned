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

package lte.apps.mec.PerformanceEvaluation.MeApp.MeAppGet_test;

import inet.applications.contract.ITCPApp;

simple MeAppGet_test like ITCPApp
{
    parameters:
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(-1); // port number to listen on
        string connectAddress = default("");  // server address (may be symbolic)
        int connectPort = default(1000); // port number to connect to
        string connectAddressApp = default("195.168.6.2");  // server address (may be symbolic)
        int connectPortApp = default(1000); // port number to connect to
		
		double meAppExecutionTime @unit(s) = default(2us);        
        
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
	gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}
