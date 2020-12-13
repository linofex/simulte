/*
 * socketManager.c
 *
 *  Created on: Dec 4, 2020
 *      Author: linofex
 */

#include "apps/mec/MeServices/MeServiceBase/SocketManager.h"
#include "common/utils/utils.h"
#include "../MeServiceBase/MeServiceBase.h"
#include "apps/mec/MeServices/httpUtils/httpUtils.h"
#include <iostream>

Register_Class(SocketManager);


void SocketManager::dataArrived(cMessage *msg, bool urgent){
        std::string packet = lte::utils::getPacketPayload(msg);
        Http::DataType type = service->getDataType(packet);
        if (type == Http::REQUEST)
        {
            //Request req = {msg, packet};
            service->newRequest(msg);
        }
        else if(type == Http::RESPONSE)
        {
            // manage depending the method used
            delete msg;
        }
        else
        {
            delete msg;
        }

}

void SocketManager::established(){
    EV_INFO << "New connection established " << endl;
}

void SocketManager::peerClosed()
{
    std::cout<<"Closed connection from: " << sock->getRemoteAddress()<< std::endl;
//    service_->removeSubscription(sock);
    sock->close();
}

void SocketManager::closed()
{
    std::cout <<"Removed socket of: " << sock->getRemoteAddress() << " from map" << std::endl;
    service->removeConnection(this);
}
void SocketManager::failure(int code)
{
    std::cout <<"Socket of: " << sock->getRemoteAddress() << " failed. Code: " << code << std::endl;

//    service_->removeSubscription(sock);
    service->removeConnection(this);
}
