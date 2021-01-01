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

#include "MEWarningAlertApp.h"

Define_Module(MEWarningAlertApp);

void MEWarningAlertApp::initialize(int stage)
{
    EV << "MEWarningAlertApp::initialize - stage " << stage << endl;

    cSimpleModule::initialize(stage);
    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    //retrieving parameters
    size_ = par("packetSize");
    ueSimbolicAddress = (char*)par("ueSimbolicAddress").stringValue();
    meHostSimbolicAddress = (char*)par("meHostSimbolicAddress").stringValue();
    destAddress_ = inet::L3AddressResolver().resolve(ueSimbolicAddress);

    //testing
    EV << "MEWarningAlertApp::initialize - MEWarningAlertApp Symbolic Address: " << meHostSimbolicAddress << endl;
    EV << "MEWarningAlertApp::initialize - UEWarningAlertApp Symbolic Address: " << ueSimbolicAddress <<  " [" << destAddress_.str() << "]" << endl;
//    socket.readDataTransferModePar(*this);
    socket.setDataTransferMode(inet::TCPDataTransferMode::TCP_TRANSFER_BYTESTREAM);

//    socket.bind(*localAddress ? inet::L3AddressResolver().resolve(localAddress) : inet::L3Address(), localPort);

    socket.setCallbackObject(this);
    socket.setOutputGate(gate("mePlatformTcpOut"));
    connect();
}

void MEWarningAlertApp::handleMessage(cMessage *msg)
{
    EV << "MEWarningAlertApp::handleMessage - \n";
    if(msg->getKind() == inet::TCP_I_DATA || msg->getKind() == inet::TCP_I_ESTABLISHED ||
       msg->getKind() == inet::TCP_I_URGENT_DATA || msg->getKind() == inet::TCP_I_CLOSED ||
       msg->getKind() == inet::TCP_I_PEER_CLOSED)
    {
        socket.processMessage(msg);
        return;
    }

    WarningAlertPacket* pkt = check_and_cast<WarningAlertPacket*>(msg);
    if (pkt == 0)
        throw cRuntimeError("MEWarningAlertApp::handleMessage - \tFATAL! Error when casting to WarningAlertPacket");

    if(!strcmp(pkt->getType(), INFO_UEAPP))         handleInfoUEWarningAlertApp(pkt);

    else if(!strcmp(pkt->getType(), INFO_MEAPP))    handleInfoMEWarningAlertApp(pkt);
}

void MEWarningAlertApp::finish(){

    EV << "MEWarningAlertApp::finish - Sending " << STOP_MEAPP << " to the MEIceAlertService" << endl;

    if(gate("mePlatformOut")->isConnected()){

        WarningAlertPacket* packet = new WarningAlertPacket();
        packet->setType(STOP_MEAPP);

        send(packet, "mePlatformOut");
    }
}
void MEWarningAlertApp::handleInfoUEWarningAlertApp(WarningAlertPacket* pkt){

    simtime_t delay = simTime()-pkt->getTimestamp();

    EV << "MEWarningAlertApp::handleInfoUEWarningAlertApp - Received: " << pkt->getType() << " type WarningAlertPacket from " << pkt->getSourceAddress() << ": delay: "<< delay << endl;

    EV << "MEWarningAlertApp::handleInfoUEWarningAlertApp - Upstream " << pkt->getType() << " type WarningAlertPacket to MEIceAlertService \n";
    send(pkt, "mePlatformOut");

    //testing
    EV << "MEWarningAlertApp::handleInfoUEWarningAlertApp position: [" << pkt->getPositionX() << " ; " << pkt->getPositionY() << " ; " << pkt->getPositionZ() << "]" << endl;
}

void MEWarningAlertApp::handleInfoMEWarningAlertApp(WarningAlertPacket* pkt){

    EV << "MEWarningAlertApp::handleInfoMEWarningAlertApp - Finalize creation of " << pkt->getType() << " type WarningAlertPacket" << endl;

    //attaching info to the ClusterizeConfigPacket created by the MEClusterizeService
    pkt->setTimestamp(simTime());
    pkt->setByteLength(size_);
    pkt->setSourceAddress(meHostSimbolicAddress);
    pkt->setDestinationAddress(ueSimbolicAddress);
    pkt->setMEModuleType("lte.apps.mec.iceAlert.MEIcerAlertApp");
    pkt->setMEModuleName("MEWarningAlertApp");

    EV << "MEWarningAlertApp::handleInfoMEWarningAlertApp - Downstream " << pkt->getType() << " type WarningAlertPacket to VirtualisationManager \n";
    send(pkt, "virtualisationInfrastructureOut");
}

 void MEWarningAlertApp::handleSelfMsg(cMessage *msg){}

void MEWarningAlertApp::connect()
{
    // we need a new connId if this is not the first connection
    socket.renewSocket();

    // connect
    const char *connectAddress = "127.0.0.1";
    int connectPort = 10020;

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

