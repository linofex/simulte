/*
 * socketManager.c
 *
 *  Created on: Dec 4, 2020
 *      Author: linofex
 */

#include "apps/mec/MeServices/MeServiceBase/SocketManager.h"
#include "common/utils/utils.h"
#include "../MeServiceBase/MeServiceBase.h"


Register_Class(SocketManager);


void SocketManager::dataArrived(cMessage *msg, bool urgent){
    service->newRequest(msg);
//    std::string packet = lte::utils::getPacketPayload(msg);
//    EV_INFO <<" dataArrived - handleRequest" << endl;
//    service->handleRequest(packet, sock);
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
