/*
 * MeAppBase.cc
 *
 *  Created on: Dec 6, 2020
 *      Author: linofex
 */


#include "apps/mec/MeApps/MeAppBase.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/httpUtils.h"
#include "common/utils/utils.h"
//simsignal_t MeAppBase::connectSignal = registerSignal("connect");
//simsignal_t MeAppBase::rcvdPkSignal = registerSignal("rcvdPk");
//simsignal_t MeAppBase::sentPkSignal = registerSignal("sentPk");

MeAppBase::MeAppBase()
{
    responseMessageLength = 0;
    receivedMessage.clear();
    sendTimer = nullptr;
    }

MeAppBase::~MeAppBase()
{
   if(sendTimer != nullptr)
   {
    if(sendTimer->isSelfMessage())
        cancelAndDelete(sendTimer);
    else
        delete sendTimer;
   }
}


void MeAppBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
//        // parameters
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        sendTimer = new cMessage("send");
        socket.readDataTransferModePar(*this);

        socket.bind(*localAddress ? inet::L3AddressResolver().resolve(localAddress) : inet::L3Address(), localPort);

        socket.setCallbackObject(this);
        socket.setOutputGate(gate("tcpOut"));

    }
}

void MeAppBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleSelfMsg(msg);
    else
        socket.processMessage(msg);
}

void MeAppBase::connect()
{
    // we need a new connId if this is not the first connection
    socket.renewSocket();

    // connect
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");

    inet::L3Address destination;
    inet::L3AddressResolver().tryResolve(connectAddress, destination);
    if (destination.isUnspecified()) {
        EV_ERROR << "Connecting to " << connectAddress << " port=" << connectPort << ": cannot resolve destination address\n";
    }
    else {
        EV_INFO << "Connecting to " << connectAddress << "(" << destination << ") port=" << connectPort << endl;

        socket.connect(destination, connectPort);

        numSessions++;
        //emit(connectSignal, 1L);
    }
}

void MeAppBase::close()
{
    EV_INFO << "issuing CLOSE command\n";
    socket.close();
    //emit(connectSignal, -1L);
}

void MeAppBase::sendPacket(cPacket *msg)
{
    int numBytes = msg->getByteLength();
    //emit(sentPkSignal, msg);
//    socket.send(msg);

    packetsSent++;
    bytesSent += numBytes;
}

void MeAppBase::refreshDisplay() const
{
    getDisplayString().setTagArg("t", 0, inet::TCPSocket::stateName(socket.getState()));
}

void MeAppBase::socketEstablished(int connId, void *)
{
    // *redefine* to perform or schedule first sending
    EV_INFO << "connected\n";
    established(connId);

}

void MeAppBase::socketDataArrived(int, void *, cPacket *msg, bool)
{
    EV << "MEClusterizeService::socketDataArrived" << endl;

    std::string packet = lte::utils::getPacketPayload(msg);
    delete msg;
    bool resp = parseReceivedMsg(packet);
//    EV << "message: " << packet << endl;
    // TODO organize code.
    // it works correctly only with 200 response COdes
    if(!resp)
    {
        EV << "MANAGE BAD MESSAGE" << endl;
        throw cRuntimeError ("qui");
        //clear all the variables
        return;
    }
    if(receivedMessage.find("Content-Length") != receivedMessage.end() && responseMessageLength == 0 )
    {
        responseMessageLength = std::stoi(receivedMessage.at("Content-Length"));
        EV << responseMessageLength << endl;
    }
    if(responseMessageLength > 0)
    {
        responseMessageLength = receivedMessage.at("body").length();
        receivingMessage = true;
    }

    if(responseMessageLength == std::stoi(receivedMessage.at("Content-Length")) && receivingMessage == true)
    {
        EV << "MEClusterizeService::socketDataArrived - Completed packet arrived." << endl;
      //  if(receivedMessage.at("type").compare("request") == 0)
            EV << "MEClusterizeService::socketDataArrived - payload: "<< endl;
            for(auto it = receivedMessage.cbegin(); it != receivedMessage.cend(); ++it)
            {
                EV << it->first << " " << it->second << endl;
            }

            handleTcpMsg();

        receivedMessage.clear();
        receivingMessage = false;
    }
    else if(responseMessageLength < 0)
        throw cRuntimeError("MEClusterizeService::socketDataArrived - read payload more than Content-Length header");
}

bool MeAppBase::parseReceivedMsg(std::string& packet)
{
    EV_INFO << "MEClusterizeService::parseResponse" << endl;

//    std::string packet(packet_);
    std::vector<std::string> splitting = lte::utils::splitString(packet, "\r\n\r\n"); // bound between header and body
    std::string header;
    std::string body;

    if(splitting.size() == 2) //header and body
    {
        EV <<"header and body" << endl;
        header = splitting[0];
        body   = splitting[1];
        receivedMessage.insert( std::pair<std::string, std::string>("body", body) );
        std::vector<std::string> line;
        std::vector<std::string> lines = lte::utils::splitString(header, "\r\n");
        std::vector<std::string>::iterator it = lines.begin();

        line = lte::utils::splitString(*it, " ");  // Request-Line e.g POST uri Http
        if(line.size() < 3 ){
           // Http::send400Response(socket);
            return false;
        }

        /* it may be a request or a response, so the first line is different
         *
         * response: HTTP/1.1 code reason
         * request:  method uri http
         *
         */

        // if it is not HTTP/1.1 or HTTP/2 (assuming the HTTP is correct ---> is is a request
        if(!Http::ceckHttpVersion(line[0]))
        {
            receivedMessage["type"]= "request";

            if(line.size() == 3)
            {
               receivedMessage.insert( std::pair<std::string, std::string>("method", line[0]));
               receivedMessage.insert( std::pair<std::string, std::string>("uri", line[1]));
               receivedMessage.insert( std::pair<std::string, std::string>("http", line[2]));
            }
            else
            {
               return false;
            }


        }
        //it a response
        else
        {
            receivedMessage["type"]= "response";
            if(line.size() == 3)
            {
                receivedMessage.insert( std::pair<std::string, std::string>("http", line[0]) );
                receivedMessage.insert( std::pair<std::string, std::string>("code", line[1]) );
                receivedMessage.insert( std::pair<std::string, std::string>("reason", line[2]) );
            }

            else if (line.size() == 4)
            {
                receivedMessage.insert( std::pair<std::string, std::string>("http", line[0]) );
                receivedMessage.insert( std::pair<std::string, std::string>("code", line[1]) );
                receivedMessage.insert( std::pair<std::string, std::string>("reason", line[2]+line[3]) );

            }
            else
            {
                return false;
            }

        }

        // read the other headers
        for(++it; it != lines.end(); ++it) {
            line = lte::utils::splitString(*it, ": ");
            if(line.size() == 2)
                receivedMessage.insert( std::pair<std::string, std::string>(line[0], line[1]) );
            else
            {
                //Http::send400Response(socket); // bad request
                return false;
            }
        }
    }
    else if(splitting.size() == 1) // only header or only body
    {

        body   = splitting[0];
        EV << "only body: " << endl;
        receivedMessage["body"] +=  body;
    }
    else
    {
        EV << "ELSE" << endl;
        return false;
    }

    return true;
}

void MeAppBase::socketPeerClosed(int, void *)
{
    // close the connection (if not already closed)
    if (socket.getState() == inet::TCPSocket::PEER_CLOSED) {
        EV_INFO << "remote TCP closed, closing here as well\n";
        close();
    }
}

void MeAppBase::socketClosed(int, void *)
{
    // *redefine* to start another session etc.
    EV_INFO << "connection closed\n";
}

void MeAppBase::socketFailure(int, void *, int code)
{
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV_WARN << "connection broken\n";
    numBroken++;
}

void MeAppBase::finish()
{
}



