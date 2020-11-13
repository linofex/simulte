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


#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "apps/mec/MeServices/packets/HTTPRespPacket.h"
#include "apps/mec/warningAlert_rest/UEWarningAlertApp_rest.h"
#include "inet/common/RawPacket.h"
#include "inet/applications/tcpapp/GenericAppMsg_m.h"


#include "apps/mec/MeServices/restServer/TCPRestSrv.h"


#include <string>
#include <vector>
#include "../../../../common/utils/utils.h"


Define_Module(TCPRestSrv);


TCPRestSrv::TCPRestSrv(){}

void TCPRestSrv::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        return;
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");


        serverSocket.setOutputGate(gate("tcpOut"));
//        serverSocket.setDataTransferMode(inet::TCP_TRANSFER_BYTESTREAM); // BYTESTREAM to send bytes!
        serverSocket.readDataTransferModePar(*this);
        serverSocket.bind(localAddress[0] ? inet::L3AddressResolver().resolve(localAddress) : inet::L3Address(), localPort);
        serverSocket.listen();

        app = check_and_cast<UEWarningAlertApp_rest*>(getModuleByPath("MecSingleCell.ue[0].udpApp[0]"));
        binder_ = getBinder();
        meHost_ = getParentModule() // virtualizationInfrastructure
                ->getParentModule(); // MeHost

        this->getConnectedEnodeB();
        L2MeasResource.addEnodeB(eNodeB_);


//        cModule *ue = getModuleByPath("MecSingleCell.ue[0].udpApp[0]");
//        EV<< "## EEE: " << app->getPosition();


        bool isOperational;
        inet::NodeStatus *nodeStatus = dynamic_cast<inet::NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == inet::NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
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
            inet::TCPSocket *socket = socketMap.findSocketFor(msg);

            if (!socket) {
                // new connection -- create new socket object and server process
                socket = new inet::TCPSocket(msg);
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
            if (msg->getKind() == inet::TCP_I_DATA || msg->getKind() == inet::TCP_I_URGENT_DATA) {
                EV << "## New packet arrived\n";
                char* packet = utils::getPacketPayload(msg);
//                inet::RawPacket *request = check_and_cast<inet::RawPacket *>(msg);
//                std::string packet(request->getByteArray().getDataPtr());
                //EV << packet;
                handleRequest(packet, socket);
            }
            else if (msg->getKind() == inet::TCP_I_PEER_CLOSED) {
                socket->close();
                socketMap.removeSocket(socket); // delete the pointer?
                delete socket;
                EV_INFO <<"Closed connection from: " << socket->getRemoteAddress();

            }
            else if (msg->getKind() == inet::TCP_I_CLOSED) {
                EV_INFO <<"Removed connection from: " << socket->getRemoteAddress();
            }
            delete msg;
           //socket->processMessage(msg); // callback
        }

}

 void TCPRestSrv::handleRequest(char* packet, inet::TCPSocket *socket){
     std::map<std::string, std::string> request = parseRequest(packet); // e.g. [0] GET [1] URI
     if(request.at("method").compare("GET") == 0)
         handleGetRequest(request.at("uri"), socket); // pass URI
     else if(request[0].compare("POST") == 0) //subscription
         handlePostRequest(request.at("uri"), request.at("body"),  socket); // pass URI
         return;
//         // TODO handle
//     else if(request[0].compare("DELETE") == 0)
//         return;
//         // TODO handle
//     else
//         throw cRuntimeError("Response code not allowed");

}

std::map<std::string, std::string> TCPRestSrv::parseRequest(char* packet_){

    std::string packet(packet_);
    std::map<std::string, std::string> headerFields;
    std::vector<std::string> splitting = utils::splitString(packet, "\r\n\r\n");
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


void TCPRestSrv::handleGetRequest(std::string& uri, inet::TCPSocket* socket){
    HTTPRespPacket temp_res = HTTPRespPacket(NULLE);

    if(uri.find("users/") != std::string::npos){
       std::string strAddress = utils::splitString(uri,"acr:")[1];

       inet::IPv4Address address(strAddress.c_str());

       MacNodeId nodeId = binder_->getMacNodeId(address);

       if(nodeId == 0){
           //riposta negativa
       }
//       inet::Coord =
       const char *moduleName = binder_->getModuleNameByMacNodeId(nodeId);
       cModule *temp = getSimulation()->getModuleByPath(moduleName)->getSubmodule("mobility");
           inet::IMobility *mobility;
           if(temp != NULL){
               mobility = check_and_cast<inet::IMobility*>(temp);
               inet::Coord position = mobility->getCurrentPosition();
               temp_res.setResCode(NULLE);
               temp_res.setContentType("application/json");
               temp_res.setConnection("keep-alive");
               temp_res.addNewLine();
//               temp_res.setBodyOK(position);

               std::string pp = L2MeasResource.toJson().dump(4);
               temp_res.setBody(pp);


           }
           else {
                   EV << "UEWarningAlertApp_rest::initialize - \tWARNING: Mobility module NOT FOUND!" << endl;
                   throw cRuntimeError("UEWarningAlertApp_rest::initialize - \tWARNING: Mobility module NOT FOUND!");
           }


    }
    // TODO define routes
//    HTTPRespPacket temp_res = HTTPRespPacket("res");
//    if(uri.find(app->getFullPath()) != std::string::npos){
//        inet::Coord position = app->getPosition();
//        // pretend all ok, generate a response
//
//        temp_res.setResCode(OK);
//        temp_res.setContentType("application/json");
//        temp_res.setConnection("keep-alive");
//        temp_res.addNewLine();
//        temp_res.setBodyOK(position);
//    }

    else{
        temp_res.setResCode(NULLE);
        temp_res.setContentType("application/json");
        temp_res.setConnection("keep-alive");
        temp_res.addNewLine();
        temp_res.setBodyNOT_FOUND("resource not found");
    }
    EV <<"\n\n\n\n######SEND!!!\n\n\n";
    inet::RawPacket *res = new RawPacket("resraw");
//    std::string s = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: keep-alive\r\n\r\n{\n\t\"DIO\" : \"BUONO\"\n}\r\n";
    res->setDataFromBuffer(temp_res.getPacket().c_str(), temp_res.getPacket().size());
    res->setByteLength(temp_res.getPacket().size());

//    e->setDataFromBuffer(s.c_str(), s.size());
//    e->setByteLength(s.size());

//    cPacket *mm = new cPacket("ciao come sta");

      socket->send(res);
//    temp_res.createRequest();
}


void TCPRestSrv::handlePostRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket){
    /*
     * check uri until resource:
     *\ /exampleAPI/location/v1/subscriptions/
    */
    if(uri.find("/exampleAPI/location/v1/subscriptions/") != std::string::npos){
        //send response with bad request
    }

    /*
     * find subscription type:
     * userTracking
     * zonalTraffic
     * zonalStatus
     */


    else{
        return;
    }




}

void TCPRestSrv::refreshDisplay() const
{
// TODO
    return;
}



void TCPRestSrv::getConnectedEnodeB(){
    int eNodeBsize = meHost_->gateSize("pppENB");
    for(int i = 0; i < eNodeBsize ; ++i){
        cModule *eNodebName = meHost_->gate("pppENB$o", i) // pppENB[i] output
                                     ->getNextGate()       // eNodeB module connected gate
                                     ->getOwnerModule();   // eBodeB module
        eNodeB_.push_back(eNodebName);
    }
    return;
}

void TCPRestSrv::finish()
{
    //TODO
    return;
}

TCPRestSrv::~TCPRestSrv(){
    socketMap.deleteSockets(); //it calls delete, too
}




