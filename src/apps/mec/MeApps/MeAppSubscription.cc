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


void MeAppSubscription::dataArrived(cPacket *msg){
    std::string packet = lte::utils::getPacketPayload(msg);
    EV_INFO << "payload: " << packet << endl;
    delete msg;

    Http::send204Response(&socket);
}

void MeAppSubscription::socketEstablished(int connId, void *yourPtr)
{

    std::string body = "{  \"L2MeasurementSubscription\": {"
               "\"callbackReference\" : \"cia/rni/v1/\","
               "\"filterCriteria\": {"
                    "\"appInstanceId\": \"01\","
                    "\"associateId\": {"
                        "\"type\": \"UE_IPv4_ADDRESS\","
                        "\"value\": \"192.168.10.1\""
                    "},"
                    "\"ecgi\":{"
                        "\"plmn\": {"
                            "\"mcc\": \"001\","
                            "\"mnc\": \"01\""
                        "},"
                        "\"cellId\": \"0\""
                   "},"
                    "\"trigger\": \"L2_MEAS_PERIODICAL\""
                "},"
                "\"expiryDeadline\": {"
                    "\"seconds\": 1577836800,"
                    "\"nanoSeconds\": 0"
                "}"
            "}"
            "}\r\n";
    std::string uri = "/example/rni/v2/subscriptions/L2_meas";
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


