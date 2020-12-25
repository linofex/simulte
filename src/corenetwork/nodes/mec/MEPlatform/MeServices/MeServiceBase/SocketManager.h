/*
 * socketManager.c
 *
 *  Created on: Dec 4, 2020
 *      Author: linofex
 */

#ifndef __SOCKETMANAGER_H
#define __SOCKETMANAGER_H

#include "corenetwork/nodes/mec/MEPlatform/MeServices/MeServiceBase/MeServiceBase.h"
#include "inet/common/INETDefs.h"
#include "inet/applications/tcpapp/TCPGenericSrvThread.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"

class SocketManager : public cObject, public TCPSocket::CallbackInterface
{
  protected:
    MeServiceBase *service;
    TCPSocket *sock;    // ptr into socketMap managed by TCPSrvHostApp

    // internal: TCPSocket::CallbackInterface methods
    virtual void socketDataArrived(int, void *, cPacket *msg, bool urgent) override { dataArrived(msg, urgent); }
    virtual void socketEstablished(int, void *) override { established(); }
    virtual void socketPeerClosed(int, void *) override { peerClosed(); }
    virtual void socketClosed(int, void *) override { closed(); }
    virtual void socketFailure(int, void *, int code) override { failure(code); }
//    virtual void socketStatusArrived(int, void *, inet::TCPStatusInfo *status) override { statusArrived(status); }

  public:

    SocketManager() { sock = nullptr; service = nullptr; }
    virtual ~SocketManager() {}

    // internal: called by TCPSrvHostApp after creating this module
    virtual void init(MeServiceBase *serv, TCPSocket *socket) { service = serv; sock = socket; }

    /*
     * Returns the socket object
     */
    virtual TCPSocket *getSocket() { return sock; }

    /*
     * Returns pointer to the host module
     */
    virtual MeServiceBase *getHostModule() { return service; }

    /**
     * Schedule an event. Do not use getContextPointer() of cMessage, because
     * TCPServerThreadBase uses it for its own purposes.
     */
//    virtual void scheduleAt(simtime_t t, cMessage *msg) { msg->setContextPointer(this); hostmod->scheduleAt(t, msg); }

    /*
     *  Cancel an event
     */
    virtual void cancelEvent(cMessage *msg) { service->cancelEvent(msg); }

    /**
     * Called when connection is established. To be redefined.
     */
    virtual void established();

    /*
     * Called when a data packet arrives. To be redefined.
     */
    virtual void dataArrived(cMessage *msg, bool urgent);

    /*
     * Called when a timer (scheduled via scheduleAt()) expires. To be redefined.
     */
//    virtual void timerExpired(cMessage *timer) = 0;

    /*
     * Called when the client closes the connection. By default it closes
     * our side too, but it can be redefined to do something different.
     */
    virtual void peerClosed();

    /*
     * Called when the connection closes (successful TCP teardown). By default
     * it deletes this thread, but it can be redefined to do something different.
     */
    virtual void closed();

    /*
     * Called when the connection breaks (TCP error). By default it deletes
     * this thread, but it can be redefined to do something different.
     */
    virtual void failure(int code);

    /*
     * Called when a status arrives in response to getSocket()->getStatus().
     * By default it deletes the status object, redefine it to add code
     * to examine the status.
     */
    virtual void statusArrived(TCPStatusInfo *status) { delete status; }
};

#endif
