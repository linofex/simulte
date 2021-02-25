/*
 * MeGetApp.h
 *
 *  Created on: Dec 6, 2020
 *      Author: linofex
 */

#ifndef _MEAPPGET_GET_H_
#define _MEAPPGET_GET_H_

#include "apps/mec/MeApps/MeAppBase.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "common/LteCommon.h"
#include "apps/mec/PerformanceEvaluation/MeApp/packets/UeAppPacket.h"

class MeAppGet_test : public MeAppBase
{
    protected:

      inet::NodeStatus *nodeStatus = nullptr;
      bool earlySend = false;    // if true, don't wait with sendRequest() until established()
      cMessage *meAppExecution_;
      double executionTime_;
      int seqNum_;
      cMessage *sendRequest_;
      double sendRequestPeriod_;
      bool waitingResponse_;


      inet::TCPSocket UeAppSocket__;

      UeAppPacket *ueAppPacket_;

//      virtual void sendRequest();
//      virtual void rescheduleOrDeleteTimer(simtime_t d, short int msgKind);
      virtual void sendRequest();


      virtual void handleSelfMsg(cMessage *msg) override;

      virtual void handleTcpMsg() override;
      virtual void established(int connId) override;

      virtual void connectUEapp();

      virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
      virtual void initialize(int stage) override;
      void handleMessage(cMessage *msg) override;
      virtual void finish() override;
//      virtual void handleTimer(cMessage *msg) override;
      virtual void socketEstablished(int connId, void *yourPtr) override;
      virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;
//      virtual void socketClosed(int connId, void *yourPtr) override;
//      virtual void socketFailure(int connId, void *yourPtr, int code) override;
//      virtual bool isNodeUp();
//      virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

      inet::Coord getCoord();
      double getOrientation();

    public:
      MeAppGet_test() {}
      virtual ~MeAppGet_test();

 };



#endif /* _MEAPPGET_GET_H_ */
