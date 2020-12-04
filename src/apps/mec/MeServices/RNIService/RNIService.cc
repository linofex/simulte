//TODO intro

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "apps/mec/warningAlert_rest/UEWarningAlertApp_rest.h"
#include "inet/common/RawPacket.h"
#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include <iostream>
#include "apps/mec/MeServices/RNIService/RNIService.h"

#include <string>
#include <vector>
//#include "apps/mec/MeServices/packets/HttpResponsePacket.h"
#include "apps/mec/MeServices/httpUtils/httpUtils.h"
#include "common/utils/utils.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"


Define_Module(RNIService);


RNIService::RNIService():L2MeasResource_(){
    baseUriQueries_ = "/example/rni/v2/queries";
    baseUriSubscriptions_ = "/example/rni/v2/subscriptions";
    subscriptionId_ = 0;
    subscriptions_.clear();
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
        L2measSubscriptionEvent_ = new cMessage("L2measSubscriptionEvent");
        L2measSubscriptionPeriod_ = par("L2measSubscriptionPeriod");

    }
}


void RNIService::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
    {
        if(strcmp(msg->getName(), "L2measSubscriptionEvent") == 0)
        {
            manageL2MeasSubscriptions();
            return;
        }

    }

    GenericService::handleMessage(msg);

}

void RNIService::handleGETRequest(const std::string& uri, inet::TCPSocket* socket)
{
    std::vector<std::string> splittedUri = lte::utils::splitString(uri, "?");
    // uri must be in form example/v1/rni/queries/resource
    std::size_t lastPart = splittedUri[0].find_last_of("/");
    if(lastPart == std::string::npos)
    {
        Http::send404Response(socket); //it is not a correct uri
        return;
    }
    // find_last_of does not take in to account if the uri has a last /
    // in this case resourceType would be empty and the baseUri == uri
    // by the way the next if statement solve this problem
    std::string baseUri = splittedUri[0].substr(0,lastPart);
    std::string resourceType =  splittedUri[0].substr(lastPart+1);

    // check it is a GET for a query or a subscription
    if(baseUri.compare(baseUriQueries_) == 0 ) //queries
    {
        if(resourceType.compare("layer2_meas") == 0 )
        {
        //look for qurery parameters
            if(splittedUri.size() == 2) // uri has parameters eg. uriPath?param=value&param1=value,value
            {
                std::vector<std::string> queryParameters = lte::utils::splitString(splittedUri[1], "&");
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
                        params = lte::utils::splitString(*it, "=");
                        if(params.size()!= 2) //must be param=values
                        {
                            Http::send400Response(socket);
                            return;
                        }
                        splittedParams = lte::utils::splitString(params[1], ",");
                        std::vector<std::string>::iterator pit  = splittedParams.begin();
                        std::vector<std::string>::iterator pend = splittedParams.end();
                        for(; pit != pend; ++pit){
                            cellIds.push_back((MacNodeId)std::stoi(*pit));
                        }
                    }
                    else if(it->rfind("ue_ipv4_address", 0) == 0)
                    {
                        params = lte::utils::splitString(*it, "=");
                        if(params.size()!= 2) //must be param=values
                        {
                            Http::send400Response(socket);
                            return;
                        }
                        splittedParams = lte::utils::splitString(params[1], ",");
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
        else
        {
            Http::send404Response(socket);
        }
    }
    else if (splittedUri[0].compare(baseUriSubscriptions_) == 0) //subs
    {
        // TODO implement subscription?
        Http::send404Response(socket);
    }
    else // not found
    {
        Http::send404Response(socket);
    }

}

void RNIService::handlePOSTRequest(const std::string& uri,const std::string& body, inet::TCPSocket* socket)
{
    // uri must be in form example/v1/rni/subscriptions/sub_type
    std::size_t lastPart = uri.find_last_of("/");
    if(lastPart == std::string::npos)
    {
        Http::send404Response(socket); //it is not a correct uri
        return;
    }

    // find_last_of does not take in to account if the uri has a last /
    // in this case subscriptionType would be empty and the baseUri == uri
    // by the way the next if statement solve this problem
    std::string baseUri = uri.substr(0,lastPart);
    std::string subscriptionType =  uri.substr(lastPart+1);

    if(baseUri.compare(baseUriSubscriptions_) == 0)
    {
        nlohmann::json jsonBody;
        try
        {
            jsonBody = nlohmann::json::parse(body); // get the JSON structure
        }
        catch(nlohmann::detail::parse_error e)
        {
            throw cRuntimeError("%s", body.c_str());
            // body is not correctly formatted in JSON, manage it
            Http::send400Response(socket); // bad body JSON
            return;
        }
        // check if the notification type
        // allowed:
        //  - L2MeasurementNotification

        SubscriptionInfo newSubscription;

        if(subscriptionType.compare("L2_meas") == 0)
        {
            // should check if the mecApp already make the subscription?
            // how? TODO
            if(jsonBody.contains("L2MeasurementSubscription")) // mandatory attribute
            {
                jsonBody = jsonBody["L2MeasurementSubscription"];// entering the structure for convenience
                if(!jsonBody.contains("callbackReference") || jsonBody["callbackReference"].is_array())
                {
                    Http::send400Response(socket); // callbackReference is mandatory and takes exactly 1 att
                    return;
                }

                if(std::string(jsonBody["callbackReference"]).find('/') == -1) //bad uri
                {
                    Http::send400Response(socket); // must be ipv4
                    return;
                }

                if(!jsonBody.contains("filterCriteria")) // mandatory attribute
                {
                    Http::send400Response(socket); // callbackReference is mandatory and takes exactly 1 att
                    return;
                }

                if(jsonBody.contains("appInstanceId") && jsonBody["appInstanceId"].is_array())
                {
                    Http::send400Response(socket); // callbackReference is mandatory and takes exactly 1 att
                    return;
                }

                if(!jsonBody.contains("filterCriteria") || jsonBody["filterCriteria"].is_array())
                {
                   Http::send400Response(socket); // filterCriteria is mandatory and takes exactly 1 att
                   return;
                }


                nlohmann::json filterCriteria = jsonBody["filterCriteria"];
                std::vector<MacNodeId> ues;
                std::vector<MacNodeId> cellids;
                std::vector<Trigger> triggers;

                //check for appInstanceId filter
                if(filterCriteria.contains("appInstanceId")  )
                {
                    if(filterCriteria["appInstanceId"].is_array())
                    {
                        Http::send400Response(socket); // appInstanceId, if present, takes exactly 1 att
                        return;
                    }
                }

                //check ues filter
                if(filterCriteria.contains("associateId"))
                {
                    if(filterCriteria["associateId"].is_array())
                    {
                        nlohmann::json ueVector = filterCriteria["associateId"];
                        for(int i = 0; i < ueVector.size(); ++i)
                        {
                            if(ueVector.at(i)["associateId"]["type"] == "UE_IPv4_ADDRESS")
                            {
                                std::string address = ueVector.at(i)["associateId"]["value"];
                                ues.push_back(binder_->getMacNodeId(IPv4Address(address.c_str())));
                            }
                            else
                            {
                                Http::send400Response(socket); // must be ipv4
                                return;
                             }
                         }
                    }
                    else
                    {
                        if(filterCriteria["associateId"]["type"] == "UE_IPv4_ADDRESS")
                        {
                            std::string address = filterCriteria["associateId"]["value"];
                            ues.push_back(binder_->getMacNodeId(IPv4Address(address.c_str())));
                        }
                    }
                }

                //check cellIds filter
                if(filterCriteria.contains("ecgi"))
                {
                    if(filterCriteria["ecgi"].is_array())
                    {
                        nlohmann::json cellVector = filterCriteria["cellId"];
                        for(int i = 0; i < cellVector.size(); ++i)
                        {
                            std::string cellId = cellVector.at(i)["cellId"];
                            cellids.push_back((MacNodeId)std::stoi(cellId));
                         }
                    }
                    else
                    {
                        std::string cellId = filterCriteria["ecgi"]["cellId"];
                        cellids.push_back((MacNodeId)std::stoi(cellId));
                    }
                }


                //check trigger filter
                if(filterCriteria.contains("trigger"))
                {
                    if(filterCriteria["trigger"].is_array())
                    {
                        nlohmann::json triggerVector = filterCriteria["trigger"];
                        for(int i = 0; i < triggerVector.size(); ++i)
                        {
                            std::string trigger = triggerVector.at(i);
                            triggers.push_back(getTrigger(trigger));
                         }
                    }
                    else
                    {
                        std::string trigger = filterCriteria["trigger"];
                        triggers.push_back(getTrigger(trigger));
                    }
                }

                //chek expiration time
                if(jsonBody.contains("expiryDeadline") && !jsonBody["expiryDeadline"].is_array())
                {
                    newSubscription.expiretaionTime = jsonBody["expiryDeadline"]["seconds"]; //TO DO add nanoseconds
                }

                newSubscription.cellIds = cellids;
                newSubscription.ues = ues;
                newSubscription.trigger = triggers;
                newSubscription.appInstanceId = filterCriteria["appInstanceId"];

                newSubscription.consumerUri = jsonBody["callbackReference"];
                newSubscription.consumerUri += "notifications/L2_meas/"+ std::to_string(subscriptionId_);
                newSubscription.socket = socket;

                std::stringstream idStream;
                idStream << "sub"<< subscriptionId_;
                newSubscription.subscriptionId = idStream.str();
                newSubscription.subscriptionType = "L2_meas";

                subscriptions_["L2_meas"][newSubscription.subscriptionId] = newSubscription;

                subscriptionId_++;
                std::string links =  host_ + baseUriSubscriptions_ + "/" + newSubscription.subscriptionId;
                nlohmann::json response;
                response["L2MeasSubscription"]["_links"]["self"] =links;
                response["L2MeasSubscription"] = jsonBody;

                std::pair<std::string, std::string> location("Location: ", links);
                //send 201 response
                Http::send201Response(socket, response.dump(2).c_str(), location);
                if(!L2measSubscriptionEvent_->isScheduled()) //subscriptions are not scheduled
                    scheduleAt(simTime() + L2measSubscriptionPeriod_, L2measSubscriptionEvent_);
                return;
            }
            else
            {
                Http::send400Response(socket); // filterCriteria is mandatory and takes exactly 1 att
                return;
            }
        }
    }
    else
    {
        Http::send404Response(socket); //resource not found
    }
}

void RNIService::handlePUTRequest(const std::string& uri,const std::string& body, inet::TCPSocket* socket){}

void RNIService::handleDELETERequest(const std::string& uri, inet::TCPSocket* socket)
{
    //get the subId from the uri
    // check the sender is the UE that created the resource
    std::size_t lastPart = uri.find_last_of("/");
    if(lastPart == std::string::npos)
    {
        throw cRuntimeError("1 - %s", uri.c_str());

        Http::send404Response(socket); //it is not a correct uri
        return;
    }

    // find_last_of does not take in to account if the uri has a last /
    // in this case subscriptionType would be empty and the baseUri == uri
    // by the way the next if statement solve this problem
    std::string subscriptionId =  uri.substr(lastPart+1);
    std::string uriAndSubType = uri.substr(0,lastPart);
    lastPart = uriAndSubType.find_last_of("/");
    if(lastPart == std::string::npos)
    {
        throw cRuntimeError("2 - %s", uriAndSubType.c_str());
        Http::send404Response(socket); //it is not a correct uri
        return;
    }
    std::string baseUri = uriAndSubType.substr(0,lastPart);
    std::string subscriptionType = uriAndSubType.substr(lastPart + 1);

    if(baseUri.compare(baseUriSubscriptions_) == 0)
    {
        SubscriptionsStructure::iterator it = subscriptions_.find(subscriptionType);
        if(it == subscriptions_.end()){
            Http::send404Response(socket);
            return;
        }
        std::map<std::string, SubscriptionInfo >::iterator rit = it->second.find(subscriptionId);
        if(rit != it->second.end()){
            it->second.erase(rit);
            if(it->second.empty() && L2measSubscriptionEvent_->isScheduled()) // no more l2meas sub, stop timer
                cancelEvent(L2measSubscriptionEvent_);

        }
        else
        {
            // manage?
        }
        Http::send204Response(socket);


    }
    else
    {

        Http::send404Response(socket); //it is not a correct uri
        return;
    }
}




void RNIService::manageL2MeasSubscriptions(){
    SubscriptionsStructure::iterator it = subscriptions_.find("L2_meas");
    if(it != subscriptions_.end())
    {
        std::map<std::string, SubscriptionInfo>::iterator sit = it->second.begin();
        while(sit != it->second.end())
        {
            std::string body;
            //send response
            if(!sit->second.ues.empty() && !sit->second.cellIds.empty())
            {
                body = L2MeasResource_.toJson(sit->second.cellIds, sit->second.ues).dump(2);
                std::cout << "ue e cell"<<std::endl;
            }
            else if(sit->second.ues.empty() && !sit->second.cellIds.empty())
            {
                body = L2MeasResource_.toJsonCell(sit->second.cellIds).dump(2);
                std::cout << "cell"<<std::endl;
            }
            else if(!sit->second.ues.empty() && sit->second.cellIds.empty())
            {
                body = L2MeasResource_.toJsonUe(sit->second.ues).dump(2);
                std::cout << "ue"<<std::endl;
            }
            else if(sit->second.ues.empty() && sit->second.cellIds.empty())
            {
                body = L2MeasResource_.toJson().dump(2);
                std::cout << "tutti"<<std::endl;
            }
            if(sit->second.socket->getState() == TCPSocket::CONNECTED)
            {
                int slash = sit->second.consumerUri.find('/');
                std::string host = sit->second.consumerUri.substr(0, slash);
                std::string uri = sit->second.consumerUri.substr(slash);
                Http::sendPostRequest(sit->second.socket, body.c_str(), host.c_str(), uri.c_str());
                sit++;
            }
            else
            { //remove the subscription since the socket is not connected
                it->second.erase(sit++);
            }

        }
        if(!it->second.empty())
            scheduleAt(simTime() + L2measSubscriptionPeriod_ , L2measSubscriptionEvent_);

    }
}
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
    cancelAndDelete(L2measSubscriptionEvent_);
return;
}




