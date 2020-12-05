// TODO intro

#include "../GenericService/GenericService.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "apps/mec/warningAlert_rest/UEWarningAlertApp_rest.h"
#include "inet/common/RawPacket.h"
#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "apps/mec/MeServices/GenericService/SocketManager.h"
#include "inet/common/INETUtils.h"
#include "common/utils/utils.h"
#include "apps/mec/MeServices/httpUtils/httpUtils.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "apps/mec/MeServices/httpUtils/json.hpp"
//Define_Module(GenericService);


GenericService::GenericService(){}

void GenericService::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        return;
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");

        requestServiceTime_ = par("requestServiceTime");
        requestService_ = new cMessage("serveRequest");

        subscriptionServiceTime_ = par("subscriptionServiceTime");
        subscriptionService_ = new cMessage("serveSubscription");


        serverSocket.setOutputGate(gate("tcpOut"));
        serverSocket.readDataTransferModePar(*this);
        serverSocket.bind(localAddress[0] ? inet::L3AddressResolver().resolve(localAddress) : inet::L3Address(), localPort);
        serverSocket.listen();


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

void GenericService::manageSubscription()
{
    cMessage *msg = check_and_cast<cMessage *>(subscriptions_.pop());
    handleSubscriptionType(msg);
    scheduleNextEvent();
}

void GenericService::scheduleNextEvent()
{
    // schedule next event
    if(subscriptions_.getLength() != 0)
        scheduleAt(simTime() + subscriptionServiceTime_ , subscriptionService_);
    else if (requests_.getLength() != 0)
        scheduleAt(simTime() + requestServiceTime_ , requestService_);
}

void GenericService::manageRequest()
{
    cMessage *msg = check_and_cast<cMessage *>(requests_.pop());
    inet::TCPSocket *socket = socketMap.findSocketFor(msg);

    if (!socket && msg->getKind() == TCP_I_ESTABLISHED) {
        // new connection -- create new socket object and server process
        socket = new inet::TCPSocket(msg);
        socket->setOutputGate(gate("tcpOut"));
        socketMap.addSocket(socket);

        std::cout <<"New connection from: " << socket->getRemoteAddress() << "tcp type " <<msg->getKind() <<std::endl ;

        const char *serverThreadClass = par("serverThreadClass");
        SocketManager *proc =
            check_and_cast<SocketManager *>(inet::utils::createOne(serverThreadClass));

        socket->setCallbackObject(proc);
        proc->init(this, socket);
    }
    if(socket)
        socket->processMessage(msg);
    else
    {
        delete msg;
    }

    scheduleNextEvent();
}


void GenericService::newRequest(cMessage *msg)
{
    int oldSize = requests_.getLength();
    requests_.insert(msg);
    // save timestamp if needed with setArrivalTime
    if(oldSize == 0 && subscriptions_.getLength() == 0) // start serving requests after subscriptions
        manageRequest();
}

void GenericService::newSubscriptionEvent(cMessage *msg)
{
    int oldSize = subscriptions_.getLength();
    subscriptions_.insert(msg);
    // save timestamp if needed with setArrivalTime
    if(oldSize == 0) // start serving subscriptions
        manageSubscription();
}

void GenericService::triggeredEvent(short int event)
{
    cMessage *msg = new cMessage("subscriptionEvent", event);
    scheduleAt(NOW, msg);
}

void GenericService::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if(strcmp(msg->getName(), "subscriptionEvent") == 0)
        {
            newSubscriptionEvent(msg);
        }
        else if(strcmp(msg->getName(), "serveSubscription") == 0)
        {
            manageSubscription();
        }
        else if(strcmp(msg->getName(), "serveRequest") == 0)
        {
            manageRequest();
        }
        else
        {
            delete msg;
        }
    }
    else { // new request arrived
        newRequest(msg);
    }

}

 void GenericService::handleRequest(std::string& packet, inet::TCPSocket *socket){
     reqMap *request = new reqMap;
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
             throw cRuntimeError ("GenericService::HTTP verb %s non recognised", request->at("method"));
     }
 }

bool GenericService::parseRequest(std::string& packet_, inet::TCPSocket *socket, reqMap* request){
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
        if(!line.empty())
            request->insert( std::pair<std::string, std::string>(line[0], line[1]) );
    }

    if(request->at("Host").compare(host_) != 0)
        return false;

    return true;
}

void GenericService::refreshDisplay() const
{
// TODO
    return;
}

void GenericService::getConnectedEnodeB(){
    int eNodeBsize = meHost_->gateSize("pppENB");
    for(int i = 0; i < eNodeBsize ; ++i){
        cModule *eNodebName = meHost_->gate("pppENB$o", i) // pppENB[i] output
                                     ->getNextGate()       // eNodeB module connected gate
                                     ->getOwnerModule();   // eBodeB module
        eNodeB_.push_back(eNodebName);
    }
    return;
}

void GenericService::finish()
{
    //TODO
    return;
}

GenericService::~GenericService(){
    socketMap.deleteSockets(); //it calls delete, too
    cancelAndDelete(requestService_);
    cancelAndDelete(subscriptionService_);
}

