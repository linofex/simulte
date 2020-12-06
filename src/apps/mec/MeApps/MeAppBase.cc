/*
 * MeAppBase.cc
 *
 *  Created on: Dec 6, 2020
 *      Author: linofex
 */


#include "apps/mec/MeApps/MeAppBase.h"
#include "inet/networklayer/common/L3AddressResolver.h"


//simsignal_t MeAppBase::connectSignal = registerSignal("connect");
//simsignal_t MeAppBase::rcvdPkSignal = registerSignal("rcvdPk");
//simsignal_t MeAppBase::sentPkSignal = registerSignal("sentPk");

void MeAppBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        numSessions = numBroken = packetsSent = packetsRcvd = bytesSent = bytesRcvd = 0;

        WATCH(numSessions);
        WATCH(numBroken);
        WATCH(packetsSent);
        WATCH(packetsRcvd);
        WATCH(bytesSent);
        WATCH(bytesRcvd);
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        // parameters
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        socket.readDataTransferModePar(*this);
        socket.bind(*localAddress ? inet::L3AddressResolver().resolve(localAddress) : inet::L3Address(), localPort);

        socket.setCallbackObject(this);
        socket.setOutputGate(gate("tcpOut"));
    }
}

void MeAppBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleSelfMsg(msg);
    else
        socket.processMessage(msg);
}

void MeAppBase::connect()
{
    // we need a new connId if this is not the first connection
    socket.renewSocket();

    // connect
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");

    inet::L3Address destination;
    inet::L3AddressResolver().tryResolve(connectAddress, destination);
    if (destination.isUnspecified()) {
        EV_ERROR << "Connecting to " << connectAddress << " port=" << connectPort << ": cannot resolve destination address\n";
    }
    else {
        EV_INFO << "Connecting to " << connectAddress << "(" << destination << ") port=" << connectPort << endl;

        socket.connect(destination, connectPort);

        numSessions++;
        //emit(connectSignal, 1L);
    }
}

void MeAppBase::close()
{
    EV_INFO << "issuing CLOSE command\n";
    socket.close();
    //emit(connectSignal, -1L);
}

void MeAppBase::sendPacket(cPacket *msg)
{
    int numBytes = msg->getByteLength();
    //emit(sentPkSignal, msg);
    socket.send(msg);

    packetsSent++;
    bytesSent += numBytes;
}

void MeAppBase::refreshDisplay() const
{
    getDisplayString().setTagArg("t", 0, inet::TCPSocket::stateName(socket.getState()));
}

void MeAppBase::socketEstablished(int, void *)
{
    // *redefine* to perform or schedule first sending
    EV_INFO << "connected\n";
}

void MeAppBase::socketDataArrived(int, void *, cPacket *msg, bool)
{
//    // *redefine* to perform or schedule next sending
//    packetsRcvd++;
//    bytesRcvd += msg->getByteLength();
//    //emit(rcvdPkSignal, msg);
//    delete msg;
//
    dataArrived(msg);

}

void MeAppBase::socketPeerClosed(int, void *)
{
    // close the connection (if not already closed)
    if (socket.getState() == inet::TCPSocket::PEER_CLOSED) {
        EV_INFO << "remote TCP closed, closing here as well\n";
        close();
    }
}

void MeAppBase::socketClosed(int, void *)
{
    // *redefine* to start another session etc.
    EV_INFO << "connection closed\n";
}

void MeAppBase::socketFailure(int, void *, int code)
{
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV_WARN << "connection broken\n";
    numBroken++;
}

void MeAppBase::finish()
{
    std::string modulePath = getFullPath();

    EV_INFO << modulePath << ": opened " << numSessions << " sessions\n";
    EV_INFO << modulePath << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";
    EV_INFO << modulePath << ": received " << bytesRcvd << " bytes in " << packetsRcvd << " packets\n";
}



