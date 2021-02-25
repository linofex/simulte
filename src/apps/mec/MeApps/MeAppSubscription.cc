/*
 * MeAppSubscription.cc
 *
 *  Created on: Dec 6, 2020
 *      Author: linofex
 */


#include "apps/mec/MeApps/MeAppSubscription.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/httpUtils.h"
#include "common/utils/utils.h"
#include <string>

Define_Module(MeAppSubscription);

MeAppSubscription::~MeAppSubscription(){}


void MeAppSubscription::established(int connId)
{
    EV << "Estabolished"<< endl;
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


