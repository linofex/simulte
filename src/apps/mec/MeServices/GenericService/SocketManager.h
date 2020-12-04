/*
 * socketManager.c
 *
 *  Created on: Dec 4, 2020
 *      Author: linofex
 */

#ifndef __SOCKETMANAGER_H
#define __SOCKETMANAGER_H

#include "inet/common/INETDefs.h"
#include "inet/applications/tcpapp/TCPGenericSrvThread.h"
class GenericService;
class TCPSocket;

class SocketManager : public inet::TCPGenericSrvThread
{
    protected:
        GenericService *service_;
    public:
        SocketManager() {}
        virtual void init(GenericService *hostmodule, inet::TCPSocket *socket);
        virtual void socketPeerClosed(int, void *) override;
        virtual void socketFailure(int, void *, int code) override;
        virtual void socketClosed(int, void *) override;

        virtual void dataArrived(cMessage *msg, bool urgent) override;

};

#endif
