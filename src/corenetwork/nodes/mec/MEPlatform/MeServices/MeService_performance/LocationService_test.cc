//TODO intro


#include "corenetwork/nodes/mec/MEPlatform/MeServices/MeService_performance/LocationService_test.h"

#include "corenetwork/nodes/mec/MEPlatform/MeServices/MeService_performance/BulkMessage.msg"


#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/MeServiceBase/SocketManager.h"
#include "inet/common/INETUtils.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/MeService_performance/FakeRequest_m.h"
#include <iostream>
#include "inet/common/RawPacket.h"
#include <string>
#include <vector>
//#include "apps/mec/MeServices/packets/HttpResponsePacket.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/httpUtils.h"
#include "common/utils/utils.h"

Define_Module(LocationService_test);


LocationService_test::LocationService_test():LocationService(){
}

void LocationService_test::initialize(int stage)
{
    if (stage == inet::INITSTAGE_LOCAL) {
        return;
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        LocationService::initialize(stage);
        queueLength = registerSignal("queueLength");
    }
}

//void LocationService_test::handleMessage(cMessage *msg)
//{
//    if (msg->isSelfMessage()) {
//           EV << "isSelfMessage" << endl;
//           if(strcmp(msg->getName(), "subscriptionEvent") == 0)
//           {
//               newSubscriptionEvent(msg);
//              // delete msg;
//           }
//           else if(strcmp(msg->getName(), "serveSubscription") == 0)
//           {
//               bool res = manageSubscription();
//               scheduleNextEvent(!res);
//           }
//           else if(strcmp(msg->getName(), "serveRequest") == 0)
//           {
//               bool res = manageRequest();
//               scheduleNextEvent(!res);
//           }
//           else
//           {
//               delete msg;
//           }
//       }
//       else { // TCP msg arrived
//
//           inet::TCPSocket *socket = socketMap.findSocketFor(msg);
//           if (!socket) // TCP_ESTABLISHED
//           {
//               socket = new inet::TCPSocket(msg);
//               socket->setOutputGate(gate("tcpOut"));
//
//
//           //        std::cout <<"New connection from: " << socket->getRemoteAddress() << " and port " << socket->getRemotePort() << std::endl ;
//               EV <<"New connection from: " << socket->getRemoteAddress() << " and port " << socket->getRemotePort() << endl ;
//
//               const char *serverThreadClass = par("serverThreadClass");
//               SocketManager *proc =
//                   check_and_cast<SocketManager *>(inet::utils::createOne(serverThreadClass));
//
//               socket->setCallbackObject(proc);
//               proc->init(this, socket);
//               socketMap.addSocket(socket);
//               socket->processMessage(msg);
//               return;
//           }
//               EV << "TCP MESSAGE" << endl;
//               socket->processMessage(msg); // it is a normal TCP message request
//       }
//}
//

double LocationService_test::calculateRequestServiceTime()
{
    EV << "LocationService_test::calculateRequestServiceTime()" << endl;
    if(strcmp(currentRequestServed_->getName(), "lastFakeRequest") == 0){
        EV << "LocationService_test::calculateRequestServiceTime - lastFakeRequest" << endl;
        double time = poisson(requestServiceTime_, REQUEST_RNG);
        return time*1e-6;;
    }
    else if(strcmp(currentRequestServed_->getName(), "fakeRequest") == 0){
        EV << "LocationService_test::calculateRequestServiceTime - fakeRequest" << endl;
        double time = poisson(requestServiceTime_, REQUEST_RNG);
        return time*1e-6;;
    }
    return MeServiceBase::calculateRequestServiceTime();
}

void LocationService_test::handleQueueFull(cMessage *msg)
{
    if(strcmp(msg->getName(), "fakeRequest") == 0)
    {
        delete msg;
        return;
    }

    MeServiceBase::handleQueueFull(msg);

}

bool LocationService_test::manageRequest()
{
    //emit(queueLength, requests_.length());
    EV << "LocationService_test::manageRequest()" << endl;
    bool result;
    //check if facke request
    if(strcmp(currentRequestServed_->getName(), "fakeRequest") == 0)
    {
        result = true;
    }
    else
    {
        inet::TCPSocket *socket = socketMap.findSocketFor(currentRequestServed_);
        if(socket)
        {
            if(strcmp(currentRequestServed_->getName(), "lastFakeRequest") == 0)
            {
                //currentRequestServed_->removeControlInfo();
                Http::send200Response(socket, "{Done}");
                //currentRequestServed_ = nullptr;
                //return true;
            }
            else
            {
                //emit(requestQueueSizeSignal_, requests_.length());
                EV_INFO <<"LocationService_test::manageRequest() - dataArrived" << endl;
                handleRequest(currentRequestServed_, socket);
                result = true;
            }
        }
        else // socket has been closed or some error occurs, discard request
        {
            // I should schedule immediately a new request execution
            result =  false;
        }
    }

    if(currentRequestServed_!= nullptr)
        delete currentRequestServed_;
    currentRequestServed_ = nullptr;
    return result;
}

LocationService_test::~LocationService_test(){
}




