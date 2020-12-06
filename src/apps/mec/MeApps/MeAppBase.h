/*
 * MeAppBase.h
 *
 *  Created on: Dec 6, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_MEAPPS_MEAPPBASE_H_
#define APPS_MEC_MEAPPS_MEAPPBASE_H_

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/common/INETDefs.h"

/**
 * Base class for clients app for TCP-based request-reply protocols or apps.
 * Handles a single session (and TCP connection) at a time.
 *
 * It needs the following NED parameters: localAddress, localPort, connectAddress, connectPort.
 */
class  MeAppBase : public cSimpleModule, public inet::TCPSocket::CallbackInterface
{
  protected:
    inet::TCPSocket socket;

//    // statistics
    int numSessions;
    int numBroken;
    int packetsSent;
    int packetsRcvd;
    int bytesSent;
    int bytesRcvd;
//
//    //statistics:
//    static simsignal_t connectSignal;
//    static simsignal_t rcvdPkSignal;
//    static simsignal_t sentPkSignal;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void dataArrived(cPacket *msg) = 0;
    virtual void handleSelfMsg(cMessage *msg) = 0;

    /* Utility functions */
    virtual void connect();
    virtual void close();
    virtual void sendPacket(cPacket *pkt);

    /* TCPSocket::CallbackInterface callback methods */
    virtual void socketEstablished(int connId, void *yourPtr) override;
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;
    virtual void socketPeerClosed(int connId, void *yourPtr) override;
    virtual void socketClosed(int connId, void *yourPtr) override;
    virtual void socketFailure(int connId, void *yourPtr, int code) override;
    virtual void socketStatusArrived(int connId, void *yourPtr, inet::TCPStatusInfo *status) override { delete status; }


};


#endif /* APPS_MEC_MEAPPS_MEAPPBASE_H_ */