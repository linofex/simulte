// TODO intro

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/common/RawPacket.h"
#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "inet/common/INETUtils.h"
#include "common/utils/utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>

#include "../MeServiceBase/MeServiceBase.h"

#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/json.hpp"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/MeServiceBase/SocketManager.h"
MeServiceBase::MeServiceBase(){}

void MeServiceBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        return;
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        EV << "Local Address: " << localAddress << " port: " << localPort << endl;

        requestServiceTime_ = par("requestServiceTime");
        requestService_ = new cMessage("serveRequest");

        subscriptionServiceTime_ = par("subscriptionServiceTime");
        subscriptionService_ = new cMessage("serveSubscription");

        currentRequestServed_ = nullptr;
        currentSubscriptionServed_ = nullptr;

        serverSocket.setOutputGate(gate("tcpOut"));
        serverSocket.readDataTransferModePar(*this);
        serverSocket.bind(localAddress[0] ? inet::L3AddressResolver().resolve(localAddress) : inet::L3Address(), localPort);
        serverSocket.listen();

        requestQueueSizeSignal_ = registerSignal("requestQueueSize");


        binder_ = getBinder();
        meHost_ = getParentModule() // virtualizationInfrastructure
                ->getParentModule(); // MeHost


        std::stringstream hostStream;
        hostStream << localAddress<< ":" << localPort;
        host_ = hostStream.str();

        this->getConnectedEnodeB();

        bool isOperational;
        inet::NodeStatus *nodeStatus = dynamic_cast<inet::NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == inet::NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void MeServiceBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        EV << "isSelfMessage" << endl;
        if(strcmp(msg->getName(), "serveSubscription") == 0)
        {
            bool res = manageSubscription();
            scheduleNextEvent(!res);
        }
        else if(strcmp(msg->getName(), "serveRequest") == 0)
        {
            bool res = manageRequest();
            scheduleNextEvent(!res);
        }
        else
        {
            delete msg;
        }
    }
    else { // TCP msg arrived

        inet::TCPSocket *socket = socketMap.findSocketFor(msg);
        if (!socket) // TCP_ESTABLISHED
        {
            socket = new inet::TCPSocket(msg);
            socket->setOutputGate(gate("tcpOut"));        requestQueueSizeSignal_ = registerSignal("requestQueueSize");



        //        std::cout <<"New connection from: " << socket->getRemoteAddress() << " and port " << socket->getRemotePort() << std::endl ;
            EV <<"New connection from: " << socket->getRemoteAddress() << " and port " << socket->getRemotePort() << endl ;

            const char *serverThreadClass = par("serverThreadClass");
            SocketManager *proc =
                check_and_cast<SocketManager *>(inet::utils::createOne(serverThreadClass));

            socket->setCallbackObject(proc);
            proc->init(this, socket);
            socketMap.addSocket(socket);
        }
        EV << "Socket Message" << endl;
        socket->processMessage(msg);
    }

}

bool MeServiceBase::manageRequest()
{
  //  EV << "MeServiceBase::manageRequest - start manageRequest" << endl;
    inet::TCPSocket *socket = socketMap.findSocketFor(currentRequestServed_);
    if(socket)
    {
        EV_INFO <<" dataArrived - handleRequest" << endl;
        handleRequest(currentRequestServed_, socket);
        if(currentRequestServed_!= nullptr)
            delete currentRequestServed_;
        currentRequestServed_ = nullptr;
        return true;
    }
    else // socket has been closed or some error occurs, discard request
    {
        // I should schedule immediately a new request execution
        if(currentRequestServed_!= nullptr)
            delete currentRequestServed_;
        return false;
    }
}

void MeServiceBase::scheduleNextEvent(bool now)
{
    // schedule next event
    if(subscriptions_.getLength() != 0 && !subscriptionService_->isScheduled())
    {
        currentSubscriptionServed_ = check_and_cast<cMessage *>(subscriptions_.pop());
        if(now)
            scheduleAt(simTime() + 0 , subscriptionService_);
        else
        {
            double time = poisson(subscriptionServiceTime_, REQUEST_RNG);
            EV <<"time: "<< time*1e-6 << endl;
            scheduleAt(simTime() + time*1e-6 , subscriptionService_);
        }
    }
    else if (requests_.getLength() != 0 && !requestService_->isScheduled() )
    {
        currentRequestServed_ = check_and_cast<cMessage *>(requests_.pop());
       // EV << "scheduleNextEvent - Request execution started" << endl;
        if(now)
            scheduleAt(simTime() + 0 , requestService_);
        else
        {
            double time = poisson(requestServiceTime_, REQUEST_RNG);
         //   EV <<"time: "<< time << "-> " <<time*1e-6 << endl;
            scheduleAt(simTime() + time*1e-6 , requestService_);
        }
    }
}

void MeServiceBase::newRequest(cMessage *msg)
{
    //EV << "Queue length: " << requests_.length() << endl;
    requests_.insert(msg);

    scheduleNextEvent();
}

void MeServiceBase::newSubscriptionEvent(cMessage *msg)
{
    subscriptions_.insert(msg);
    scheduleNextEvent();
}

bool MeServiceBase::manageSubscription()
{
    return handleSubscriptionType(currentSubscriptionServed_);

}

void MeServiceBase::triggeredEvent(short int event)
{
    //chiamo direttamente la newSubscriptionEvent
    cMessage *msg = new cMessage("subscriptionEvent", event);
    newSubscriptionEvent(msg);
}

void MeServiceBase::handleRequest(cMessage* msg, inet::TCPSocket *socket){
     reqMap *request = new reqMap;
     std::string packet = lte::utils::getPacketPayload(msg);
     bool res = parseRequest(packet, socket, request); // e.g. [0] GET [1] URI

     if(res){ // request-line is well formatted

         if(request->at("method").compare("GET") == 0)
             handleGETRequest(request->at("uri"), socket); // pass URI
         else if(request->at("method").compare("POST") == 0) //subscription
             handlePOSTRequest(request->at("uri"), request->at("body"),  socket); // pass URI
         else if(request->at("method").compare("PUT") == 0)
             handlePUTRequest(request->at("uri"), request->at("body"),  socket); // pass URI

         else if(request->at("method").compare("DELETE") == 0)
             handleDELETERequest(request->at("uri"),  socket); // pass URI
         else if(request->at("method").compare("HEAD") == 0)
             Http::send405Response(socket);

         else if(request->at("method").compare("CONNECT") == 0)
             Http::send405Response(socket);

         else if(request->at("method").compare("TRACE") == 0)
             Http::send405Response(socket);

         else if(request->at("method").compare("PATCH") == 0)
             Http::send405Response(socket);

         else if(request->at("method").compare("OPTIONS") == 0)
             Http::send405Response(socket);
         else
             throw cRuntimeError ("MeServiceBase::HTTP verb %s non recognised", request->at("method"));
     }
     delete request;
 }

bool MeServiceBase::parseRequest(std::string& packet_, inet::TCPSocket *socket, reqMap* request){
    EV_INFO << "parseRequest - Start parseRequest" << endl;
//    std::string packet(packet_);
    std::vector<std::string> splitting = lte::utils::splitString(packet_, "\r\n\r\n"); // bound between header and body
    std::string header;
    std::string body;
    
    if(splitting.size() == 2)
    {
        header = splitting[0];
        body   = splitting[1];
        request->insert( std::pair<std::string, std::string>("body", body) );
    }
    else if(splitting.size() == 1) // no body
    {
        header = splitting[0];
    }
    else //incorrect request
    {
       Http::send400Response(socket); // bad request
       return false;
    }
    
    std::vector<std::string> line;
    std::vector<std::string> lines = lte::utils::splitString(header, "\r\n");
    std::vector<std::string>::iterator it = lines.begin();
    line = lte::utils::splitString(*it, " ");  // Request-Line GET / HTTP/1.1
    if(line.size() != 3 ){
        Http::send400Response(socket);
        return false;
    }
    if(!Http::ceckHttpVersion(line[2])){
        Http::send505Response(socket);
        return false;
    }

    request->insert( std::pair<std::string, std::string>("method", line[0]) );
    request->insert( std::pair<std::string, std::string>("uri", line[1]) );
    request->insert( std::pair<std::string, std::string>("http", line[2]) );

    for(++it; it != lines.end(); ++it) {
        line = lte::utils::splitString(*it, ": ");
        if(line.size() == 2)
            request->insert( std::pair<std::string, std::string>(line[0], line[1]) );
        else
        {
            Http::send400Response(socket); // bad request
            return false;
        }
    }
    if(request->at("Host").compare(host_) != 0)
        return false;
    return true;
}

void MeServiceBase::refreshDisplay() const
{
// TODO
    return;
}

void MeServiceBase::getConnectedEnodeB(){
    int eNodeBsize = meHost_->gateSize("pppENB");
    for(int i = 0; i < eNodeBsize ; ++i){
        cModule *eNodebName = meHost_->gate("pppENB$o", i) // pppENB[i] output
                                     ->getNextGate()       // eNodeB module connected gate
                                     ->getOwnerModule();   // eBodeB module
        eNodeB_.push_back(eNodebName);
    }
    return;
}


void MeServiceBase::removeConnection(SocketManager *connection)
{
    // remove socket
    socketMap.removeSocket(connection->getSocket());
    // remove thread object
    delete connection;
}
void MeServiceBase::finish()
{
    //TODO
    return;
}

MeServiceBase::~MeServiceBase(){
    socketMap.deleteSockets(); //it calls delete, too
    cancelAndDelete(requestService_);
    cancelAndDelete(subscriptionService_);
    cancelAndDelete(currentRequestServed_);
    cancelAndDelete(currentSubscriptionServed_);

    cObject* msg;
    while(!requests_.isEmpty())
    {
        msg = requests_.pop();
        delete msg;
    }

    while(!subscriptions_.isEmpty())
    {
        msg = subscriptions_.pop();
        delete msg;

    }
}
void MeServiceBase::emitRequestQueueLength()
{
    emit(requestQueueSizeSignal_, requests_.getLength());
}


Http::DataType MeServiceBase::getDataType(std::string& packet_){
    // HTTP request or HTTP response
    if (packet_.rfind("HTTP", 0) == 0) { // is a response
        // parse it
        return Http::RESPONSE;
    }
    else if(packet_.rfind("GET", 0) == 0 || packet_.rfind("POST", 0) == 0 ||
            packet_.rfind("DELETE", 0) == 0 || packet_.rfind("PUT", 0) == 0)
    {
        return Http::REQUEST;
        // is a request
    }
    else
    {
        return Http::UNKNOWN;
    }

}











