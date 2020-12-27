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


void MeAppGet::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent){
    std::string packet = lte::utils::getPacketPayload(msg);
    EV_INFO << "payload: " << packet << endl;
    delete msg;
    close();
}

void MeAppGet::socketEstablished(int connId, void *yourPtr)
{

    std::string body = "";
    std::string uri = "/example/location/v2/queries/users";
    std::string host = socket.getRemoteAddress().str()+":"+std::to_string(socket.getRemotePort());
    Http::sendGetRequest(&socket, body.c_str(), host.c_str(), uri.c_str());

}


void MeAppGet::handleSelfMsg(cMessage *msg){
    if(strcmp(msg->getName(), "connect") == 0)
    {
        connect();
        delete msg;
        return;
    }
}

void MeAppGet::initialize(int stage){
    MeAppBase::initialize(stage);
    if(stage == inet::INITSTAGE_APPLICATION_LAYER)
    {
        cMessage *m = new cMessage("connect");
        scheduleAt(simTime()+0.000012, m);
    }
}


