/*
 * MeApp_test.h
 *
 *  Created on: Dec 13, 2020
 *      Author: linofex
 */


/*
 * This module simulates the presence of n Mec Application requesting
 * something from the service.
 *
 * This allow the simulation to run faster, since there is not the need to
 * manage a huge amount of events.
 *
 * The number of applications X is specified via .ini file.
 * The application sends a unique msg containing the number of applications, to
 * simulate X requests.
 * Since the requests follow the request-response method, a new bulk message will be
 * sent only after the reception of a message from the service, that notify the response to the
 * last message.
 * Then, the application can send a new bulk of request.
 */

#ifndef APPS_MEC_MEAPPS_MEAPP_TEST_H_
#define APPS_MEC_MEAPPS_MEAPP_TEST_H_


#include "apps/mec/MeApps/MeAppBase.h"
#include "inet/common/lifecycle/NodeStatus.h"



class MeApp_test : public MeAppBase
{
    protected:

      inet::NodeStatus *nodeStatus = nullptr;
      int numberOfApplications_;    // requests to send in this session


//      virtual void sendRequest();
//      virtual void rescheduleOrDeleteTimer(simtime_t d, short int msgKind);

      virtual void socketDataArrived(int, void *, cPacket *msg, bool) override;
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

      virtual void sendBulkRequest();

    public:
      MeApp_test() {}
      virtual ~MeApp_test();

 };



#endif /* APPS_MEC_MEAPPS_MEAPP_TEST_H_ */
