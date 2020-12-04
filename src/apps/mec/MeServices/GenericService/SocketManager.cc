/*
 * socketManager.c
 *
 *  Created on: Dec 4, 2020
 *      Author: linofex
 */

#include "apps/mec/MeServices/GenericService/SocketManager.h"
#include "common/utils/utils.h"
#include "../GenericService/GenericService.h"


Register_Class(SocketManager);

void SocketManager::init(GenericService *hostmodule, inet::TCPSocket *socket) {
    service_ = hostmodule;
    sock = socket;
}

void SocketManager::dataArrived(cMessage *msg, bool urgent){
    std::string packet = lte::utils::getPacketPayload(msg);
    service_->handleRequest(packet, sock);
    delete msg;
}

void SocketManager::socketPeerClosed(int, void *)
{
    std::cout<<"Closed connection from: " << sock->getRemoteAddress()<< std::endl;
//    service_->removeSubscription(sock);
    sock->close();
}

void SocketManager::socketClosed(int, void *)
{
    std::cout <<"Removed socket of: " << sock->getRemoteAddress() << " from map" << std::endl;
    service_->removeThread(this);
}
void SocketManager::socketFailure(int, void *, int code)
{
    std::cout <<"Socket of: " << sock->getRemoteAddress() << " failed. Code: " << code << std::endl;

//    service_->removeSubscription(sock);
    service_->removeThread(this);
}
