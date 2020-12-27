// 

package lte.apps.mec.MeApp_test;

import inet.applications.contract.ITCPApp;

simple MeApp_test like ITCPApp
{
    parameters:
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(-1); // port number to listen on
        string connectAddress = default("");  // server address (may be symbolic)
        int connectPort = default(1000); // port number to connect to
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
		
		int numberOfApplications;

    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}
