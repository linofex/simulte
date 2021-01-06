/*
 * MeApp_test.cc
 *
 *  Created on: Dec 6, 2020
 *      Author: linofex
 */


#include "apps/mec/MeApp_test/MeApp_test.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/httpUtils.h"
#include "common/utils/utils.h"
#include <string>
#include "corenetwork/nodes/mec/MEPlatform/MeServices/MeService_performance/BulkMessage_m.h"
#include "inet/common/RawPacket.h"
Define_Module(MeApp_test);

MeApp_test::~MeApp_test(){}


void MeApp_test::handleTcpMsg()
{
    EV_INFO << "payload: " <<endl;//<< receivedMessage.at("body") << endl;
    sendBulkRequest();
}

void MeApp_test::established(int connId)
{
    sendBulkRequest();
}


void MeApp_test::handleSelfMsg(cMessage *msg){
    if(strcmp(msg->getName(), "connect") == 0)
    {
        connect();
        delete msg;
        return;
    }
    if(strcmp(msg->getName(), "response") == 0)
    {
        delete msg;
        // maybe add a delay
        sendBulkRequest();
    }
}

void MeApp_test::initialize(int stage){
    MeAppBase::initialize(stage);
    if(stage == inet::INITSTAGE_APPLICATION_LAYER)
    {
        numberOfApplications_ = par("numberOfApplications");
        cMessage *m = new cMessage("connect");
        scheduleAt(simTime() + 0.055, m);
    }
}

void MeApp_test::sendBulkRequest(){
    inet::RawPacket *request = new inet::RawPacket("BulkRequest");
    std::string payload = "BulkRequest: " + std::to_string(numberOfApplications_);
    request->setDataFromBuffer(payload.c_str(), payload.length());
    request->setByteLength(payload.length());
    socket.send(request);
    EV << "sent"<<endl;
}



