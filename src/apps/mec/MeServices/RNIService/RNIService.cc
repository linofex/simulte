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


//#include "apps/mec/MeServices/RNIService/RNIService.h"
//
//#include "inet/networklayer/common/L3AddressResolver.h"
//#include "inet/common/ModuleAccess.h"
//#include "inet/common/lifecycle/NodeStatus.h"
//#include "inet/transportlayer/contract/tcp/TCPSocket.h"
//#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
//#include "apps/mec/MeServices/packets/HTTPRespPacket.h"
//#include "inet/common/RawPacket.h"
//#include "inet/applications/tcpapp/GenericAppMsg_m.h"
//
//
//#include "apps/mec/MeServices/utils/utils.h"
//#include <string>
//#include <vector>



#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "apps/mec/MeServices/packets/HTTPRespPacket.h"
#include "apps/mec/warningAlert_rest/UEWarningAlertApp_rest.h"
#include "inet/common/RawPacket.h"
#include "inet/applications/tcpapp/GenericAppMsg_m.h"


#include "apps/mec/MeServices/RNIService/RNIService.h"


#include <string>
#include <vector>
#include "../../../../common/utils/utils.h"


Define_Module(RNIService);


RNIService::RNIService():L2MeasResource(){}

void RNIService::initialize(int stage)
{
    cSimpleModule::initialize(stage);


    if (stage == inet::INITSTAGE_LOCAL) {
        return;
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        GenericService::initialize(stage);
        L2MeasResource.addEnodeB(eNodeB_);
    }
}


void RNIService::handleMessage(cMessage *msg)
{
    GenericService::handleMessage(msg);
}
//
// void RNIService::handleRequest(char* packet, inet::TCPSocket *socket){
//     std::map<std::string, std::string> request = parseRequest(packet); // e.g. [0] GET [1] URI
//     if(request.at("method").compare("GET") == 0)
//         handleGetRequest(request.at("uri"), socket); // pass URI
//     else if(request[0].compare("POST") == 0) //subscription
//         handlePostRequest(request.at("uri"), request.at("body"),  socket); // pass URI
//         return;
////         // TODO handle
////     else if(request[0].compare("DELETE") == 0)
////         return;
////         // TODO handle
////     else
////         throw cRuntimeError("Response code not allowed");
//
//}
//
//std::map<std::string, std::string> RNIService::parseRequest(char* packet_){
//
//    std::string packet(packet_);
//    std::map<std::string, std::string> headerFields;
//    std::vector<std::string> splitting = utils::splitString(packet, "\r\n\r\n"); // bound between header and body
//    std::string header = splitting[0];
//    std::string body   = splitting[1];
//
//    std::vector<std::string> line;
//    int i = 0;
//    std::vector<std::string> lines = utils::splitString(header, "\r\n");
//    for(std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
//         if(i++ == 0){ // Request-Line GET / HTTP/1.1
//             line = utils::splitString(*it, " ");
//             if(line.size() != 3){
//                 //BADREQUEST
//                 //exit
//             }
//             headerFields["method"] = line[0];
//             headerFields["uri"] = line[1];
//             headerFields["http"] = line[2];
//         }
//         else{
//             line = utils::splitString(*it, ": ");
//             if(!line.empty())
//                 headerFields[line[0]] = line[1]; //fieldname -> value
//             }
//    }
//    //TODO also return the body
//    return headerFields; // maybe better return a pointer
//}
//
void RNIService::handleGETRequest(const std::string& uri, inet::TCPSocket* socket){
    HTTPRespPacket temp_res = HTTPRespPacket(OK);

    if(uri.find("users/") != std::string::npos){
       std::string strAddress = utils::splitString(uri,"acr:")[1];

       inet::IPv4Address address(strAddress.c_str());

       MacNodeId nodeId = binder_->getMacNodeId(address);

       if(nodeId == 0){
           //riposta negativa
       }
       const char *moduleName = binder_->getModuleNameByMacNodeId(nodeId);
       cModule *temp = getSimulation()->getModuleByPath(moduleName)->getSubmodule("mobility");
           inet::IMobility *mobility;
           if(temp != NULL){
               mobility = check_and_cast<inet::IMobility*>(temp);
               inet::Coord position = mobility->getCurrentPosition();
               temp_res.setResCode(BAD_METHOD);
               temp_res.setContentType("application/json");
               temp_res.setConnection("keep-alive");
               temp_res.addNewLine();

               std::string pp = L2MeasResource.toJson().dump(4);
               temp_res.setBody(pp);


           }
           else {
                   EV << "UEWarningAlertApp_rest::initialize - \tWARNING: Mobility module NOT FOUND!" << endl;
                   throw cRuntimeError("UEWarningAlertApp_rest::initialize - \tWARNING: Mobility module NOT FOUND!");
           }


    }


    EV <<"\n\n\n\n######SEND!!!\n\n\n";
    inet::RawPacket *res = new RawPacket("resraw");
    res->setDataFromBuffer(temp_res.getPacket().c_str(), temp_res.getPacket().size());
    res->setByteLength(temp_res.getPacket().size());

    socket->send(res);

}


void RNIService::handlePOSTRequest(const std::string& uri,const std::string& body, inet::TCPSocket* socket){}

    void RNIService::handlePUTRequest(const std::string& uri,const std::string& body, inet::TCPSocket* socket){}

        void RNIService::handleDELETERequest(const std::string& uri,const std::string& body, inet::TCPSocket* socket){}

//
//
//void RNIService::handlePostRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket){
//    /*
//     * check uri until resource:
//     *\ /exampleAPI/location/v1/subscriptions/
//    */
//    if(uri.find("/exampleAPI/location/v1/subscriptions/") != std::string::npos){
//        //send response with bad request
//    }
//
//    /*
//     * find subscription type:
//     * userTracking
//     * zonalTraffic
//     * zonalStatus
//     */
//
//
//    else{
//        return;
//    }
//
//
//
//
//}

void RNIService::finish()
{
// TODO
    return;
}


void RNIService::refreshDisplay() const
{
// TODO
    return;
}


RNIService::~RNIService(){
return;
}




