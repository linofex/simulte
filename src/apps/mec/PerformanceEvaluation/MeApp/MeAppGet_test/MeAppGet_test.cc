/*
 * MeAppGet_test.cc
 *
 *  Created on: Dec 6, 2020
 *      Author: linofex
 */


#include "apps/mec/PerformanceEvaluation/MeApp/MeAppGet_test/MeAppGet_test.h"
#include "apps/mec/MeServices/httpUtils/httpUtils.h"
#include "common/utils/utils.h"
#include <string>
#include "inet/networklayer/common/L3AddressResolver.h"
#include "apps/mec/MeServices/httpUtils/json.hpp"
#include "corenetwork/lteCellInfo/LteCellInfo.h"
#include "corenetwork/binder/LteBinder.h"

#include "inet/mobility/base/MovingMobilityBase.h"

Define_Module(MeAppGet_test);

MeAppGet_test::~MeAppGet_test()
{
    cancelAndDelete(meAppExecution_);
    cancelAndDelete(sendRequest_);

//    if(ueAppPacket_ != nullptr)
//        delete ueAppPacket_;
}

inet::Coord MeAppGet_test::getCoord()
{
//    LteBinder* temp = getBinder();
//    MacNodeId id = temp->getMacNodeId(inet::IPv4Address("10.0.0.1"));
//    LteCellInfo *cell = getCellInfo(id);
//    return cell->getUePosition(id);
       LteBinder* temp = getBinder();
       MacNodeId id = temp->getMacNodeId(inet::IPv4Address("10.0.0.1"));
       OmnetId omnetId = temp->getOmnetId(id);
       omnetpp::cModule* module = getSimulation()->getModule(omnetId);
       inet::MovingMobilityBase *mobility_ = check_and_cast<inet::MovingMobilityBase *>(module->getSubmodule("mobility"));
       return mobility_->getCurrentPosition();

}


double MeAppGet_test::getOrientation()
{
    LteBinder* temp = getBinder();
    MacNodeId id = temp->getMacNodeId(inet::IPv4Address("10.0.0.1"));
    OmnetId omnetId = temp->getOmnetId(id);
    omnetpp::cModule* module = getSimulation()->getModule(omnetId);

    inet::MovingMobilityBase *mobility_ = check_and_cast<inet::MovingMobilityBase *>(module->getSubmodule("mobility"));
    return mobility_->getCurrentSpeed().x;

}



void MeAppGet_test::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{}


void MeAppGet_test::sendRequest()
{
    EV << " send get request "<< seqNum_<< " position " << getCoord().x << endl;
    ueAppPacket_ = new UeAppPacket("ueAppPacket");
    ueAppPacket_->setRequestTime(simTime().dbl());
    ueAppPacket_->setRequestCoord(getCoord());
    ueAppPacket_->setRequestOrientation(getOrientation());
    ueAppPacket_->setSeqNum(seqNum_++);
    std::string body = "";
    std::string uri = "/example/location/v2/queries/users?address=acr:10.0.0.1";
    std::string host = socket.getRemoteAddress().str()+":"+std::to_string(socket.getRemotePort());
    Http::sendGetRequest(&socket, body.c_str(), host.c_str(), uri.c_str());
    waitingResponse_ = true;
    scheduleAt(simTime() + sendRequestPeriod_, sendRequest_);
}

void MeAppGet_test::socketEstablished(int connId, void *yourPtr)
{
      if(connId == socket.getConnectionId())
      {
          EV << "send get request" << endl;
      }
      else
      {
          EV<< "Ue app connection established" << endl;
          sendRequest();
      }

}


void MeAppGet_test::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleSelfMsg(msg);
    else
    {
        inet::TCPCommand *ind = dynamic_cast<inet::TCPCommand *>(msg->getControlInfo());
        if (!ind)
            throw cRuntimeError("TCPSocketMap: findSocketFor(): no TCPCommand control info in message (not from TCP?)");

        int connId = ind->getConnId();
        if (msg->getKind() == inet::TCP_I_DATA || msg->getKind() == inet::TCP_I_URGENT_DATA)
        {
            if(connId == socket.getConnectionId()) // service responded
            {
                if(ueAppPacket_ == nullptr)
                    throw cRuntimeError("MeAppGet_test::handleMessage - ueAppPacket must implemented");
                ueAppPacket_->setResponseTime(simTime().dbl());
                std:: string payload = lte::utils::getPacketPayload(msg);
                //EV<< "p" << payload;
                inet::Coord respCoord;
                nlohmann::json jsonBody;
                jsonBody = nlohmann::json::parse(lte::utils::splitString(payload, "\r\n\r\n")[1]); // get the JSON structure
                respCoord.x = jsonBody["userInfo"]["locationInfo"]["x"];
                respCoord.y = jsonBody["userInfo"]["locationInfo"]["y"];
                respCoord.z = jsonBody["userInfo"]["locationInfo"]["z"];
                ueAppPacket_->setResponseCoord(respCoord);
                ueAppPacket_->setResponseOrientation(jsonBody["userInfo"]["locationInfo"]["speed"]["x"]);

                EV<< "data received for request:  " << ueAppPacket_->getSeqNum() << " position " <<  respCoord.x << endl;
                waitingResponse_ = false;
                scheduleAt(simTime() + executionTime_, meAppExecution_);
                delete msg;

            }
            else // data message from UE app
            {return;}
        }
        else
        {
            if(connId == socket.getConnectionId())
                socket.processMessage(msg);
            else
                UeAppSocket__.processMessage(msg);
        }

    }
}

void MeAppGet_test::handleSelfMsg(cMessage *msg){
    if(strcmp(msg->getName(), "connect") == 0)
    {
        connectUEapp();
        connect();

        delete msg;
        return;
    }
    if(strcmp(msg->getName(), "meAppExecution") == 0)
    {
        EV<< "sent packet with seqNum: "<< ueAppPacket_->getSeqNum() << " to Ue app" << endl;
        ueAppPacket_->setByteLength(64);
        UeAppSocket__.send(ueAppPacket_);
        if(!sendRequest_->isScheduled())
            scheduleAt(simTime()+sendRequestPeriod_, sendRequest_);
        return;
    }
    if(strcmp(msg->getName(), "sendRequest") ==  0)
    {
        if(waitingResponse_ == false)
            sendRequest();
    }

}

void MeAppGet_test::initialize(int stage){
    MeAppBase::initialize(stage);
    if(stage == inet::INITSTAGE_APPLICATION_LAYER)
    {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        UeAppSocket__.setDataTransferMode(inet::TCP_TRANSFER_OBJECT);
        UeAppSocket__.bind(*localAddress ? inet::L3AddressResolver().resolve(localAddress) : inet::L3Address(), localPort);
        UeAppSocket__.setCallbackObject(this);
        UeAppSocket__.setOutputGate(gate("tcpOut"));
        ueAppPacket_ = nullptr;
        meAppExecution_ = new cMessage("meAppExecution");
        executionTime_ = par("meAppExecutionTime");


        cMessage *m = new cMessage("connect");
        scheduleAt(simTime()+0.500013, m);

        sendRequest_ = new cMessage("sendRequest");
        sendRequestPeriod_ = 0.025;
        waitingResponse_ = false;
        seqNum_ = 0;
    }
}


void MeAppGet_test::connectUEapp()
{
    // we need a new connId if this is not the first connection
    UeAppSocket__.renewSocket();

    // connect
    const char *connectAddress = par("connectAddressApp");
    int connectPort = par("connectPortApp");

    inet::L3Address destination;
    inet::L3AddressResolver().tryResolve(connectAddress, destination);
    if (destination.isUnspecified()) {
        EV_ERROR << "Connecting to " << connectAddress << " port=" << connectPort << ": cannot resolve destination address\n";
    }
    else {
        EV_INFO << "Connecting to " << connectAddress << "(" << destination << ") port=" << connectPort << endl;

        UeAppSocket__.connect(destination, connectPort);

        numSessions++;
        //emit(connectSignal, 1L);
    }
}



void MeAppGet_test::finish()
{

    socket.close();
    UeAppSocket__.close();
}
