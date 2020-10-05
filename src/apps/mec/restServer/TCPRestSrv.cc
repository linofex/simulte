//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/applications/restServer/TCPRestSrv.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/applications/restServer/packets/HTTPRespPacket.h"
#include "inet/applications/restServer/packets/RestPacket.h"

#include "inet/common/RawPacket.h"
#include "inet/applications/restServer/utils/utils.h"


#include <string>
#include <vector>

namespace inet {

Define_Module(TCPRestSrv);


void TCPRestSrv::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        return;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");

        serverSocket.setOutputGate(gate("tcpOut"));
        serverSocket.setDataTransferMode(TCP_TRANSFER_BYTESTREAM); // BYTESTREAM to send bytes!
        serverSocket.bind(localAddress[0] ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
        serverSocket.listen();
        EV<<"###IN LTE";
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}


void TCPRestSrv::sendResponse(cMessage *msg, TCPSocket *socket)
{
    cPacket *packet = dynamic_cast<cPacket *>(msg);

    socket->send(msg);
}


//void  TCPRestSrv::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) {
//    EV<<"\n########ECCOLO:  "<< msg;
//}
void TCPRestSrv::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        delete msg;
    }
    else {
            TCPSocket *socket = socketMap.findSocketFor(msg);

            if (!socket) {
                // new connection -- create new socket object and server process
                socket = new TCPSocket(msg);
                socket->setOutputGate(gate("tcpOut"));

//                const char *serverThreadClass = par("serverThreadClass");
//                TCPServerThreadBase *proc =
//                    check_and_cast<TCPServerThreadBase *>(inet::utils::createOne(serverThreadClass));
//
//                socket->setCallbackObject(this); // to use callback
//                proc->init(this, socket);

                socketMap.addSocket(socket);
                EV_INFO <<"New connection from: " << socket->getRemoteAddress();
                delete msg;
                return;
            }


            if (msg->getKind() == TCP_I_DATA || msg->getKind() == TCP_I_URGENT_DATA) {
                EV << "## New packet arrived\n";
                RawPacket *request = check_and_cast<RawPacket *>(msg);
                char *packet = request->getByteArray().getDataPtr();
                EV << packet;
                handleRequest(packet, socket);
            }
            else if (msg->getKind() == TCP_I_PEER_CLOSED) {
                socket->close();
                socketMap.removeSocket(socket); // delete the pointer?
                delete socket;
                EV_INFO <<"Closed connection from: " << socket->getRemoteAddress();

            }
            else if (msg->getKind() == TCP_I_CLOSED) {
                EV_INFO <<"Removed connection from: " << socket->getRemoteAddress();
            }
            delete msg;
           //socket->processMessage(msg); // callback
        }

}

 void TCPRestSrv::handleRequest(char *packet, TCPSocket *socket){
     std::map<std::string, std::string> request = parseRequest(packet); // e.g. [0] GET [1] URI
     if(request.at("method").compare("GET") == 0)
         handleGetRequest(request.at("uri"), socket); // pass URI
//     else if(request[0].compare("POST") == 0)
//         return;
//         // TODO handle
//     else if(request[0].compare("DELETE") == 0)
//         return;
//         // TODO handle
//     else
//         throw cRuntimeError("Response code not allowed");

}

std::map<std::string, std::string> TCPRestSrv::parseRequest(char *packet_){

    std::string tre(packet_);
   // std::string packet(packet_);
    std::map<std::string, std::string> headerFields;
    std::vector<std::string> splitting = utils::splitString(tre, "\r\n\r\n");
    std::string header = splitting[0];
    std::string body   = splitting[1];


    std::vector<std::string> line;
    size_t last = 0;
    size_t next = 0;
    int i = 0;
    std::vector<std::string> lines = utils::splitString(header, "\r\n");
    for(std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
         if(i++ == 0){ //fisrt line e.g. GET / HTTP/1.1
             line = utils::splitString(*it, " ");
             if(line.size() != 3){
                 //BADREQUEST
                 //exit
             }
             headerFields["method"] = line[0];
             headerFields["uri"] = line[1];
             headerFields["http"] = line[2];
         }
         else{
             line = utils::splitString(*it, ": ");
             if(!line.empty())
                 headerFields[line[0]] = line[1]; //fieldname -> value
             }
    }
    return headerFields; // maybe better return a pointer
}


void TCPRestSrv::handleGetRequest(std::string uri, TCPSocket* socket){
    // pretend all ok, generate a response
//    HTTPRespPacket *res = new HTTPRespPacket("res");
//    res->setHeaderField("ciao");
//    res->createRequest();
    //delete res;
    EV <<"\n\n\n\n######SEND!!!\n\n\n";
    RawPacket *e = new RawPacket("resraw");
    std::string s = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: keep-alive\r\n\r\n{\n\t\"Hello\" : \"World\"\n}\r\n";
    e->setDataFromBuffer(s.c_str(), s.size());
    e->setByteLength(s.size());

    socket->send(e);
//    res->createRequest();
}


void TCPRestSrv::refreshDisplay() const
{
// TODO
}

void TCPRestSrv::finish()
{
}

TCPRestSrv::~TCPRestSrv(){
    socketMap.deleteSockets(); //it calls delete, too
}


} // namespace inet

