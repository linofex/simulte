/*
 * socketManager.c
 *
 *  Created on: Dec 4, 2020
 *      Author: linofex
 */

#include "corenetwork/nodes/mec/MEPlatform/MeServices/MeService_performance/SocketManager_normal.h"
#include "common/utils/utils.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/httpUtils.h"
#include <iostream>

Register_Class(SocketManager_normal);


void SocketManager_normal::dataArrived(cMessage *msg, bool urgent){
        std::string packet = lte::utils::getPacketPayload(msg);
        Http::DataType type = service->getDataType(packet);
        if (type == Http::REQUEST)
        {
            int requests = service->getQueuedApp();
            if(requests < 0)
              throw cRuntimeError("Number of request must be non negative");
            EV << " of size "<< requests << endl;
            for(int i = 0 ; i < requests ; ++i)
            {
                  cMessage *request = new cMessage("fakeRequest");
                  service->newRequest(request);

            }
            //Request req = {msg, packet};
            service->newRequest(msg);
        }
        else if(type == Http::RESPONSE)
        {
            // manage depending the method used
            delete msg;
        }

}

void SocketManager_normal::established(){
    EV_INFO << "New connection established " << endl;
}

void SocketManager_normal::peerClosed()
{
    std::cout<<"Closed connection from: " << sock->getRemoteAddress()<< std::endl;
//    service_->removeSubscription(sock);
    sock->close();
}

void SocketManager_normal::closed()
{
    std::cout <<"Removed socket of: " << sock->getRemoteAddress() << " from map" << std::endl;
    service->removeConnection(this);
}
void SocketManager_normal::failure(int code)
{
    std::cout <<"Socket of: " << sock->getRemoteAddress() << " failed. Code: " << code << std::endl;

//    service_->removeSubscription(sock);
    service->removeConnection(this);
}
