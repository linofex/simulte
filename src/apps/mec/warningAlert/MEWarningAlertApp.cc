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
#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/httpUtils.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/json.hpp"
#include "common/utils/utils.h"

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

    responseMessageLength = 0;
    receivingMessage = false;


}

void MEWarningAlertApp::handleMessage(cMessage *msg)
{
//   / EV << "MEWarningAlertApp::handleMessage - \n";
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

    //if(!strcmp(pkt->getType(), INFO_UEAPP))         handleInfoUEWarningAlertApp(pkt);

    //else if(!strcmp(pkt->getType(), INFO_MEAPP))    handleInfoMEWarningAlertApp(pkt);
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

void MEWarningAlertApp::handleTcpMsg(){

    if(receivedMessage.at("type").compare("request") == 0)
    {
        nlohmann::json jsonBody;
        EV << "MEClusterizeService::handleTcpMsg - OK " << receivedMessage.at("body")<< endl;
        try
        {

           jsonBody = nlohmann::json::parse(receivedMessage.at("body")); // get the JSON structure
        }
        catch(nlohmann::detail::parse_error e)
        {
           EV <<  e.what() << endl;
           // body is not correctly formatted in JSON, manage it
           return;
        }

        if(jsonBody.contains("subscriptionNotification"))
        {
            if(jsonBody["subscriptionNotification"].contains("enteringLeavingCriteria"))
            {
                nlohmann::json criteria = jsonBody["subscriptionNotification"]["enteringLeavingCriteria"] ;
                if(criteria == "Entering")
                {
                    //send subscription for leaving..
                    WarningAlertPacket* packet = new WarningAlertPacket();
                    packet->setType(INFO_MEAPP);
                    packet->setDanger(true);
                    send(packet, "virtualisationInfrastructureOut");
                    modifySubscription();

                }
                else if (criteria == "Leaving")
                {
                    WarningAlertPacket* packet = new WarningAlertPacket();
                    packet->setType(INFO_MEAPP);
                    packet->setDanger(false);
                    send(packet, "virtualisationInfrastructureOut");
                }
            }
        }
    }
    else
    {
        if(receivedMessage.at("code").compare("201") == 0)
        {
            nlohmann::json jsonBody;
            EV << "MEClusterizeService::handleTcpMsg - OK " << receivedMessage.at("body")<< endl;
            try
            {

               jsonBody = nlohmann::json::parse(receivedMessage.at("body")); // get the JSON structure
            }
            catch(nlohmann::detail::parse_error e)
            {
               EV <<  e.what() << endl;
               // body is not correctly formatted in JSON, manage it
               return;
            }
            std::string resourceUri = jsonBody["circleNotificationSubscription"]["resourceURL"];
            std::size_t lastPart = resourceUri.find_last_of("/");
            if(lastPart == std::string::npos)
            {
                EV << "1" << endl;
                return;
            }
            // find_last_of does not take in to account if the uri has a last /
            // in this case subscriptionType would be empty and the baseUri == uri
            // by the way the next if statement solve this problem
            std::string baseUri = resourceUri.substr(0,lastPart);
            //save the id
            subId = resourceUri.substr(lastPart+1);
            EV << "subId: " << subId << endl;
        }
    }


}

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

void MEWarningAlertApp::modifySubscription()
{
    //    sendSubscription("Entering");
        std::string body = "{  \"circleNotificationSubscription\": {"
                           "\"callbackReference\" : {"
                            "\"callbackData\":\"1234\","
                            "\"notifyURL\":\"example.com/notification/1234\"},"
                           "\"checkImmediate\": \"false\","
                            "\"address\": \"" + destAddress_.str()+ "\","
                            "\"clientCorrelator\": \"ciao\","
                            "\"enteringLeavingCriteria\": \"Leaving\","
                            "\"frequency\": 10,"
                            "\"radius\": 60,"
                            "\"trackingAccuracy\": 10,"
                            "\"latitude\": 210,"
                            "\"longitude\": 260"
                            "}"
                            "}\r\n";
                std::string uri = "/example/location/v2/subscriptions/area/circle/" + subId;
                std::string host = socket.getRemoteAddress().str()+":"+std::to_string(socket.getRemotePort());
                Http::sendPutRequest(&socket, body.c_str(), host.c_str(), uri.c_str());


}
void MEWarningAlertApp::established(int connId)
{
//    sendSubscription("Entering");
    std::string body = "{  \"circleNotificationSubscription\": {"
                       "\"callbackReference\" : {"
                        "\"callbackData\":\"1234\","
                        "\"notifyURL\":\"example.com/notification/1234\"},"
                       "\"checkImmediate\": \"false\","
                        "\"address\": \"" + destAddress_.str()+ "\","
                        "\"clientCorrelator\": \"ciao\","
                        "\"enteringLeavingCriteria\": \"Entering\","
                        "\"frequency\": 10,"
                        "\"radius\": 60,"
                        "\"trackingAccuracy\": 10,"
                        "\"latitude\": 210,"
                        "\"longitude\": 260"
                        "}"
                        "}\r\n";
            std::string uri = "/example/location/v2/subscriptions/area/circle";
            std::string host = socket.getRemoteAddress().str()+":"+std::to_string(socket.getRemotePort());
            Http::sendPostRequest(&socket, body.c_str(), host.c_str(), uri.c_str());
}



void MEWarningAlertApp::handleSelfMsg(cMessage *msg){}
