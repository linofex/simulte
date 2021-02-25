/*
 * MeAppGet.cc
 *
 *  Created on: Dec 6, 2020
 *      Author: linofex
 */


#include "apps/mec/MeApps/MeAppGet.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/httpUtils.h"
#include "common/utils/utils.h"
#include <string>

Define_Module(MeAppGet);

MeAppGet::~MeAppGet(){}


void MeAppGet::handleTcpMsg()
{
    EV_INFO << "payload: " << receivedMessage.at("body") << endl;
    //emit(responseTime_, simTime() - sendTimestamp);
    if(burstFlag == true)
           scheduleAt(simTime() +exponential(0.01, 2),sendBurst);

}

void MeAppGet::sendMsg(){
//    std::string body = "";
//    std::string uri = "/example/location/v2/queries/users";
//    std::string host = socket.getRemoteAddress().str()+":"+std::to_string(socket.getRemotePort());
//    Http::sendGetRequest(&socket, body.c_str(), host.c_str(), uri.c_str());
//    sendTimestamp = simTime();

    inet::RawPacket *request = new inet::RawPacket("BulkRequest");

     //}while(numRequests == 0);
     std::string payload = "BulkRequest: 1";
     request->setDataFromBuffer(payload.c_str(), payload.length());
     request->setByteLength(payload.length());
     socket.send(request);
}
void MeAppGet::established(int connId)
{
//    sendMsg();
    scheduleAt(simTime() + 5,burstTimer);
}

void MeAppGet::handleSelfMsg(cMessage *msg){
    if(strcmp(msg->getName(), "connect") == 0)
    {
        connect();
        delete msg;
        return;
    }
    else if(strcmp(msg->getName(), "send") == 0)
    {
        sendMsg();
    }
    if(strcmp(msg->getName(), "burstTimer") == 0)
    {
        burstFlag = (burstFlag == true)? false : true;
        if(burstFlag){
            sendMsg();
            scheduleAt(simTime()+0.5, burstPeriod);
        }
        scheduleAt(simTime()+5,burstTimer);
    }
    if(strcmp(msg->getName(), "burstPeriod") == 0)
        {
            burstFlag = false;
//            if(burstFlag)
//                scheduleAt(simTime()+0.5, burstPeriod);
        }
    if(strcmp(msg->getName(), "sendBurst") == 0)
    {
        sendMsg();
    }




}

void MeAppGet::initialize(int stage){
    MeAppBase::initialize(stage);
    if(stage == inet::INITSTAGE_APPLICATION_LAYER)
    {
        cMessage *m = new cMessage("connect");
        scheduleAt(simTime()+0.000012, m);
        sendBurst = new cMessage("sendBurst");
        burstPeriod = new cMessage("burstPeriod");
        burstTimer = new cMessage("burstTimer");
        burstFlag = false;

        responseTime_ = registerSignal("responseTime");
    }
}


