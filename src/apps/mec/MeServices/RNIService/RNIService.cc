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
#include "common/utils/utils.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"


Define_Module(RNIService);


RNIService::RNIService():L2MeasResource_(){
    baseUriQueries_ = "/example/rni/v2/queries";
    baseUriSubscriptions_ = "/example/rni/v2/subscriptions";
    supportedQueryParams_.insert("cell_id");
    supportedQueryParams_.insert("ue_ipv4_address");
    // supportedQueryParams_s_.insert("ue_ipv6_address");
}

void RNIService::initialize(int stage)
{
    cSimpleModule::initialize(stage);


    if (stage == inet::INITSTAGE_LOCAL) {
        return;
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        GenericService::initialize(stage);
        L2MeasResource_.addEnodeB(eNodeB_);
    }
}


void RNIService::handleMessage(cMessage *msg)
{
    GenericService::handleMessage(msg);
}

void RNIService::handleGETRequest(const std::string& uri, inet::TCPSocket* socket){
    std::vector<std::string> splittedUri = utils::splitString(uri, "?");
    // check it is a GET for a query or a subscription
    if(splittedUri[0] == baseUriQueries_) //queries
    {
        //look for qurery parameters
        if(splittedUri.size() == 2) // uri has parameters eg. uriPath?param=value&param1=value,value
        {
            std::vector<std::string> queryParameters = utils::splitString(splittedUri[1], "&");
            /*
            * supported paramater:
            * - cell_id
            * - ue_ipv4_address
            * - ue_ipv6_address // not implemented yet
            */

            std::vector<MacNodeId> cellIds;
            std::vector<MacNodeId> ues;

            typedef std::map<std::string, std::vector<std::string>> queryMap;
            queryMap queryParamsMap; // e.g cell_id -> [0, 1]

            std::vector<std::string>::iterator it  = queryParameters.begin();
            std::vector<std::string>::iterator end = queryParameters.end();
            std::vector<std::string> params;
            std::vector<std::string> splittedParams;
            for(; it != end; ++it){
                if(it->rfind("cell_id", 0) == 0) // cell_id=par1,par2
                {
                    params = utils::splitString(*it, "=");
                    if(params.size()!= 2) //must be param=values
                    {
                        Http::send400Response(socket);
                        return;
                    }
                    splittedParams = utils::splitString(params[1], ",");
                    std::vector<std::string>::iterator pit  = splittedParams.begin();
                    std::vector<std::string>::iterator pend = splittedParams.end();
                    for(; pit != pend; ++pit){
                        cellIds.push_back((MacNodeId)std::stoi(*pit));
                    }
                }
                else if(it->rfind("ue_ipv4_address", 0) == 0)
                {
                    params = utils::splitString(*it, "=");
                    if(params.size()!= 2) //must be param=values
                    {
                        Http::send400Response(socket);
                        return;
                    }
                    splittedParams = utils::splitString(params[1], ",");
                    std::vector<std::string>::iterator pit  = splittedParams.begin();
                    std::vector<std::string>::iterator pend = splittedParams.end();
                    for(; pit != pend; ++pit){
                       //manage ipv4 address without any macnode id
                        //or do the conversion inside L2Meas..
                       ues.push_back(binder_->getMacNodeId(IPv4Address((*pit).c_str())));
                    }
                }
                else // bad parameters
                {
                    Http::send400Response(socket);
                    return;
                }

            }

            //send response
            if(!ues.empty() && !cellIds.empty())
            {
                Http::send200Response(socket, L2MeasResource_.toJson(cellIds, ues).dump(2).c_str());
            }
            else if(ues.empty() && !cellIds.empty())
            {
                Http::send200Response(socket, L2MeasResource_.toJsonCell(cellIds).dump(2).c_str());
            }
            else if(!ues.empty() && cellIds.empty())
           {
               Http::send200Response(socket, L2MeasResource_.toJsonUe(ues).dump(2).c_str());
           }
           else
           {
               Http::send400Response(socket);
           }

        }
        else if (splittedUri.size() == 1 ){ //no query params
            Http::send200Response(socket,L2MeasResource_.toJson().dump(2).c_str());
            return;
        }
        else //bad uri
        {
            Http::send404Response(socket);
        }
    }
    else if (splittedUri[0] == baseUriSubscriptions_) //subs
    {
        // TODO implement subscription?
        Http::send404Response(socket);
    }
    else // not found
    {
        Http::send404Response(socket);
    }

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




