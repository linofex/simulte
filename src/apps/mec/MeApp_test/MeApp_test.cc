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
    //double time = exponential(0.01, 2);
    if(burstFlag)
        scheduleAt(simTime() + 0, sendBurst);

}

void MeApp_test::established(int connId)
{
//    sendBulkRequest();
    EV << "Established " << endl;
    scheduleAt(simTime() + 5, burstTimer);
}


void MeApp_test::handleSelfMsg(cMessage *msg){
    if(strcmp(msg->getName(), "connect") == 0)
    {
        connect();
        delete msg;
        return;
    }
    if(strcmp(msg->getName(), "burstTimer") == 0)
    {
        burstFlag = (burstFlag == true)? false : true;
        if(burstFlag){
            sendBulkRequest();
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
        sendBulkRequest();
    }

}

void MeApp_test::initialize(int stage){
    MeAppBase::initialize(stage);
    if(stage == inet::INITSTAGE_APPLICATION_LAYER)
    {
        numberOfApplications_ = par("numberOfApplications");

        if(numberOfApplications_ != 0)
        {
            cMessage *m = new cMessage("connect");
            sendBurst = new cMessage("sendBurst");
            burstPeriod = new cMessage("burstPeriod");
            burstTimer = new cMessage("burstTimer");
            burstFlag = false;
            scheduleAt(simTime() + 0.055, m);
        }
    }
}

void MeApp_test::sendBulkRequest(){
    inet::RawPacket *request = new inet::RawPacket("BulkRequest");
    int numRequests;
    //do{

//        numRequests = poisson(numberOfApplications_, 2);
    numRequests = truncnormal(numberOfApplications_, 20, 2);

    //}while(numRequests == 0);
    std::string payload = "BulkRequest: " + std::to_string(numberOfApplications_+1);
    request->setDataFromBuffer(payload.c_str(), payload.length());
    request->setByteLength(payload.length());
    socket.send(request);
    EV << "sent " << numRequests << " requests to the server"<<endl;
}



