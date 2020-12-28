/*
 * MeAppSubscription.cc
 *
 *  Created on: Dec 6, 2020
 *      Author: linofex
 */


#include "apps/mec/MeApps/MeAppSubscription.h"
#include "apps/mec/MeServices/httpUtils/httpUtils.h"
#include "common/utils/utils.h"
#include <string>

Define_Module(MeAppSubscription);

MeAppSubscription::~MeAppSubscription(){}


void MeAppSubscription::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
    std::string packet = lte::utils::getPacketPayload(msg);
    EV_INFO << "payload: " << packet << endl;
    delete msg;

    Http::send204Response(&socket);
}

void MeAppSubscription::socketEstablished(int connId, void *yourPtr)
{

    std::string body = "{  \"circleNotificationSubscription\": {"
               "\"callbackReference\" : {"
                "\"callbackData\":\"1234\","
                "\"notifyURL\":\"Ssss4\"},"
               "\"checkImmediate\": \"true\","
            "\"clientCorrelator\": \"ciao\","
            "\"enteringLeavingCriteria\": \"Entering\","
            "\"frequency\": 10,"
            "\"radius\": 10,"
            "\"trackingAccuracy\": 10,"
            "\"latitude\": 10,"
            "\"longitude\": 10"
            "}"
            "}\r\n";
    std::string uri = "/example/location/v2/subscriptions/area/circle";
    std::string host = socket.getRemoteAddress().str()+":"+std::to_string(socket.getRemotePort());
    Http::sendPostRequest(&socket, body.c_str(), host.c_str(), uri.c_str());

}


void MeAppSubscription::handleSelfMsg(cMessage *msg){
    if(strcmp(msg->getName(), "connect") == 0)
    {
        connect();
        delete msg;
        return;
    }
}

void MeAppSubscription::initialize(int stage){
    MeAppBase::initialize(stage);
    if(stage == inet::INITSTAGE_APPLICATION_LAYER)
    {
        cMessage *m = new cMessage("connect");
        scheduleAt(simTime()+0.5, m);
    }
}


