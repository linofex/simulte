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
#include "apps/mec/MeServices/packets/HttpResponsePacket.h"
#include "apps/mec/warningAlert_rest/UEWarningAlertApp_rest.h"
#include "inet/common/RawPacket.h"
#include "inet/applications/tcpapp/GenericAppMsg_m.h"

#include "apps/mec/MeServices/RNIService/RNIService.h"


#include <string>
#include <vector>
#include "apps/mec/MeServices/packets/HttpResponsePacket.h"
#include "apps/mec/MeServices/httpUtils/httpUtils.h"

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

void RNIService::handleGETRequest(const std::string& uri, inet::TCPSocket* socket){
//    inet::RawPacket *res = new RawPacket("resraw");
//    std::string e = L2MeasResource.toJson().dump(2);
//
//
//        res->setDataFromBuffer(e.c_str(), e.size());
//        res->setByteLength(e.size());
//
//        socket->send(res);
//
    Http::send200Response(socket, L2MeasResource.toJson().dump(2).c_str());

//
//    HTTPRespPacket temp_res = HTTPRespPacket(OK);
//
//    if(uri.find("users/") != std::string::npos){
//       std::string strAddress = utils::splitString(uri,"acr:")[1];
//
//       inet::IPv4Address address(strAddress.c_str());
//
//       MacNodeId nodeId = binder_->getMacNodeId(address);
//
//       if(nodeId == 0){
//           //riposta negativa
//       }
//       const char *moduleName = binder_->getModuleNameByMacNodeId(nodeId);
//       cModule *temp = getSimulation()->getModuleByPath(moduleName)->getSubmodule("mobility");
//           inet::IMobility *mobility;
//           if(temp != NULL){
//               mobility = check_and_cast<inet::IMobility*>(temp);
//               inet::Coord position = mobility->getCurrentPosition();
//               temp_res.setResCode(BAD_METHOD);
//               temp_res.setContentType("application/json");
//               temp_res.setConnection("keep-alive");
//               temp_res.addNewLine();
//
//               std::string pp = L2MeasResource.toJson().dump(4);
//               temp_res.setBody(pp);
//
//
//           }
//           else {
//                   EV << "UEWarningAlertApp_rest::initialize - \tWARNING: Mobility module NOT FOUND!" << endl;
//                   throw cRuntimeError("UEWarningAlertApp_rest::initialize - \tWARNING: Mobility module NOT FOUND!");
//           }
//
//
//    }
//
//
//    EV <<"\n\n\n\n######SEND!!!\n\n\n";
//    inet::RawPacket *res = new RawPacket("resraw");
//    res->setDataFromBuffer(temp_res.getPacket().c_str(), temp_res.getPacket().size());
//    res->setByteLength(temp_res.getPacket().size());
//
//    socket->send(res);

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




