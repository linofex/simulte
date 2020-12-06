/*
 * MeAppGet.cc
 *
 *  Created on: Dec 6, 2020
 *      Author: linofex
 */


#include "apps/mec/MeApps/MeAppGet.h"


Define_Module(MeAppGet);

MeAppGet::~MeAppGet(){}


void MeAppGet::dataArrived(cPacket *msg){}
void MeAppGet::handleSelfMsg(cMessage *msg){
    connect();
    delete msg;
}

void MeAppGet::initialize(int stage){
    MeAppBase::initialize(stage);
    if(stage == inet::INITSTAGE_APPLICATION_LAYER)
    {
        cMessage *m = new cMessage("ciao");
        scheduleAt(simTime()+1, m);
    }
}


