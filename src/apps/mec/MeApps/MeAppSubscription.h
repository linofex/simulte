/*
 * MeAppSubscription.h
 *
 *  Created on: Dec 10, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_MEAPPS_MEAPPSUBSCRIPTION_H_
#define APPS_MEC_MEAPPS_MEAPPSUBSCRIPTION_H_

#include "apps/mec/MeApps/MeAppBase.h"
#include "inet/common/lifecycle/NodeStatus.h"

class MeAppSubscription : public MeAppBase
{
    protected:
    int i;
      inet::NodeStatus *nodeStatus = nullptr;
      bool earlySend = false;    // if true, don't wait with sendRequest() until established()
      int numRequestsToSend = 0;    // requests to send in this session
      simtime_t startTime;
      simtime_t stopTime;

//      virtual void sendRequest();
//      virtual void rescheduleOrDeleteTimer(simtime_t d, short int msgKind);

      virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;
      virtual void handleSelfMsg(cMessage *msg);

      virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
      virtual void initialize(int stage) override;
//      virtual void handleTimer(cMessage *msg) override;
      virtual void socketEstablished(int connId, void *yourPtr) override;
//      virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;
//      virtual void socketClosed(int connId, void *yourPtr) override;
//      virtual void socketFailure(int connId, void *yourPtr, int code) override;
//      virtual bool isNodeUp();
//      virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    public:
      MeAppSubscription() {i = 0;}
      virtual ~MeAppSubscription();

 };




#endif /* APPS_MEC_MEAPPS_MEAPPSUBSCRIPTION_H_ */
