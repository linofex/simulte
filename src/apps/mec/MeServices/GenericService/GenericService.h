// TODO intro
//

#ifndef __INET_GENERICSERVICE_H
#define __INET_GENERICSERVICE_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"
#include <vector>
#include <map>
#include "inet/applications/tcpapp/TCPSrvHostApp.h"
#include <queue>
#include "corenetwork/binder/LteBinder.h"
#include "common/MecCommon.h"

/**
 *
 *
 *
 */

typedef std::map<std::string, std::string> reqMap;


class GenericService: public inet::TCPSrvHostApp
{
    public:
        GenericService();

    protected:
//        inet::TCPSocket serverSocket; // Used to listen incoming connections
//        inet::TCPSocketMap socketMap; // Stores the connections
        std::string host_;
        LteBinder* binder_;
        cModule* meHost_;
        std::vector<cModule*> eNodeB_;     //eNodeBs connected to the ME Host

        cMessage *requestService_;
        double requestServiceTime_;
        cQueue requests_;

        cMessage *subscriptionService_;
        double subscriptionServiceTime_;
        cQueue subscriptions_;


        virtual void manageRequest();
        virtual void manageSubscription(); // the trigger depends on the resource
        virtual void newRequest(cMessage *msg);
        virtual void newSubscriptionEvent(cMessage *msg);

        virtual void scheduleNextEvent();

        virtual void initialize(int stage) override;
        virtual int  numInitStages() const override { return inet::NUM_INIT_STAGES; }
        virtual void handleMessage(cMessage *msg) override;
        virtual void finish() override;
        virtual void refreshDisplay() const override;
        virtual void getConnectedEnodeB();

        virtual bool parseRequest(std::string&, inet::TCPSocket *socket, reqMap* request);


        virtual void handleGETRequest(const std::string& uri, inet::TCPSocket* socket) = 0;
        virtual void handlePOSTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket)   = 0;
        virtual void handlePUTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket)    = 0;
        virtual void handleDELETERequest(const std::string& uri, inet::TCPSocket* socket) = 0;
        virtual void handleSubscriptionType(cMessage *msg) = 0;

//        virtual void removeSubscription(inet::TCPSocket* socket) = 0;
        virtual ~GenericService();

        virtual bool handleOperationStage(inet::LifecycleOperation *operation, int stage, inet::IDoneCallback *doneCallback) override
        { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

    public:
        virtual void triggeredEvent(short int event);
        virtual void handleRequest(std::string& packet, inet::TCPSocket *socket);

};


#endif // ifndef __INET_GENERICSERVICE_H

