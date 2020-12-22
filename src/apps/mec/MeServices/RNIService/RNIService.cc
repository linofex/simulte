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

#include "apps/mec/MeServices/Resources/SubscriptionBase.h"
Define_Module(RNIService);


RNIService::RNIService():L2MeasResource_(){
    baseUriQueries_ = "/example/rni/v2/queries";
    baseUriSubscriptions_ = "/example/rni/v2/subscriptions";
    subscriptionId_ = 0;
    subscriptions_.clear();
    supportedQueryParams_.insert("cell_id");
    supportedQueryParams_.insert("ue_ipv4_address");
    scheduledSubscription = false;
    // supportedQueryParams_s_.insert("ue_ipv6_address");
}

void RNIService::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        return;
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        MeServiceBase::initialize(stage);
        L2MeasResource_.addEnodeB(eNodeB_);
        L2measSubscriptionEvent_ = new cMessage("L2measSubscriptionEvent");
        L2measSubscriptionPeriod_ = par("L2measSubscriptionPeriod");
        baseSubscriptionLocation_ = host_+ baseUriSubscriptions_ + "/";
    }
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
    // uri must be in form example/v1/rni/subscriptions/
//    std::size_t lastPart = uri.find_last_of("/");
//    if(lastPart == std::string::npos)
//    {
//
//    }
//
//    // find_last_of does not take in to account if the uri has a last /
//    // in this case subscriptionType would be empty and the baseUri == uri
//    // by the way the next if statement solve this problem
//    std::string baseUri = uri.substr(0,lastPart);
//    std::string subscriptionType =  uri.substr(lastPart+1);

    if(uri.compare(baseUriSubscriptions_) == 0)
    {

        nlohmann::json jsonBody;
        try
        {
            jsonBody = nlohmann::json::parse(body); // get the JSON structure
        }
        catch(nlohmann::detail::parse_error e)
        {
            std::cout <<  e.what() << std::endl;
            // body is not correctly formatted in JSON, manage it
            Http::send400Response(socket); // bad body JSON
            return;
        }
        // check the subscription type
        // allowed:
        //  - L2MeasurementSubscription
        //  - cellChangeSubscription

        std::string subscription;


//        if(subscriptionType.compare("cell_change") == 0)
//        {
////            // set id
////            int subId;
////            CellChangeSubscription newSubscription = CellChangeSubscription();
////            bool res =  newSubscription.parse(id, socket, jsonBody);
////            if(res)
////            {
////
////                cellSubscriptions_[Id]  = newSubscription;
////                subid++;
////                cell->addCellChangeNotification();
////
////
////            }
//
//            //TODO
//
//        }
//
        if(jsonBody.contains("MeasRepUeSubscription"))
        {
            MeasRepUeSubscription newSubscription = MeasRepUeSubscription(subscriptionId_, socket , baseSubscriptionLocation_,  eNodeB_);
            MeasRepUeSubscription* ee = new MeasRepUeSubscription(subscriptionId_, socket , baseSubscriptionLocation_,  eNodeB_);

            bool res = newSubscription.fromJson(jsonBody);
            //correct subscription post
            if(res)
            {
                std::cout << "ok" << std::endl;
                subscriptionId_ ++;
                measRepUeSubscriptions_[subscriptionId_] = newSubscription;
                subscriptions_[subscriptionId_] = ee;
                socketToSubId_[socket].insert(subscriptionId_);
                subscriptionId_ ++;
            }

        }

        else if(jsonBody.contains("L2MeasurementSubscription"))
        {
            L2MeasSubscription newSubscription = L2MeasSubscription(subscriptionId_, socket , baseSubscriptionLocation_,  eNodeB_);
            bool res = newSubscription.fromJson(jsonBody);
            //correct subscription post
            if(res)
            {
                std::cout << "ok" << std::endl;
                l2MeasSubscriptions_[subscriptionId_] = newSubscription;
                socketToSubId_[socket].insert(subscriptionId_);
                subscriptionId_ ++;
            }

        }
    }
    else
    {
        Http::send404Response(socket); //resource not found
        return;
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

    std::string baseUri = uri.substr(0,lastPart);
    unsigned int subscriptionId =  std::stoul(uri.substr(lastPart+1)); // e.g sub132


    if(baseUri.compare(baseUriSubscriptions_) == 0)
    {
        // check if the subId it is present
        if(l2MeasSubscriptions_.find(subscriptionId) != l2MeasSubscriptions_.end())
        {
            l2MeasSubscriptions_.erase(subscriptionId);
            socketToSubId_[socket].erase(subscriptionId);
            Http::send204Response(socket);
        }
        else if(measRepUeSubscriptions_.find(subscriptionId) != measRepUeSubscriptions_.end())
        {
            measRepUeSubscriptions_.erase(subscriptionId);
            socketToSubId_[socket].erase(subscriptionId);
            Http::send204Response(socket);
        }
        else
        {
            Http::send404Response(socket);
        }

    }
    else
    {

        Http::send404Response(socket); //it is not a correct uri
        return;
    }
}

bool RNIService::handleSubscriptionType(cMessage *msg)
{
    if(msg->getKind() == L2_MEAS_PERIODICAL)
    {
        delete msg;
        return manageL2MeasSubscriptions(L2_MEAS_PERIODICAL);
    }
    else if(msg->getKind() == UE_CQI)
    {
        delete msg;
    }

}

bool RNIService::manageL2MeasSubscriptions(Trigger trigger){
//    SubscriptionsStructure::iterator it = subscriptions_.find("L2_meas");
//    bool sent = false;
//    if(it != subscriptions_.end())
//    {
//        std::map<std::string, SubscriptionInfo>::iterator sit = it->second.begin();
//        while(sit != it->second.end())
//        {
//            std::string body;
//            //send response
//            if(sit->second.trigger == trigger)
//            {
//                if(trigger == L2_MEAS_PERIODICAL)
//                {
//                    if(!sit->second.ues.empty() && !sit->second.cellIds.empty())
//                    {
//                        body = L2MeasResource_.toJson(sit->second.cellIds, sit->second.ues).dump(2);
//                        std::cout << "ue e cell"<<std::endl;
//                    }
//                    else if(sit->second.ues.empty() && !sit->second.cellIds.empty())
//                    {
//                        body = L2MeasResource_.toJsonCell(sit->second.cellIds).dump(2);
//                        std::cout << "cell"<<std::endl;
//                    }
//                    else if(!sit->second.ues.empty() && sit->second.cellIds.empty())
//                    {
//                        body = L2MeasResource_.toJsonUe(sit->second.ues).dump(2);
//                        std::cout << "ue"<<std::endl;
//                    }
//                    else if(sit->second.ues.empty() && sit->second.cellIds.empty())
//                    {
//                        body = L2MeasResource_.toJson().dump(2);
//                        std::cout << "tutti"<<std::endl;
//                    }
//                }
//                else if(trigger == L2_MEAS_UTI_80)
//                {
//                    // tutte le toJson con id cella
//                }
//    //                body = L2MeasResource_.toJsonCell("utilization").dump(2);
//                }
//            if(sit->second.socket->getState() == TCPSocket::CONNECTED)
//            {
//                if(body.size() > 0)
//                {
//                    int slash = sit->second.consumerUri.find('/');
//                    std::string host = sit->second.consumerUri.substr(0, slash);
//                    std::string uri = sit->second.consumerUri.substr(slash);
//                    Http::sendPostRequest(sit->second.socket, body.c_str(), host.c_str(), uri.c_str());
//                    sent = true;
//                }
//            sit++;
//
//            }
//            else
//            { //remove the subscription since the socket is not connected
//                it->second.erase(sit++);
//            }

//        }

//        if(!it->second.empty())
//        {
//            cMessage *msg = new cMessage("subscriptionEvent", L2_MEAS_PERIODICAL);
//            scheduleAt(simTime() + L2measSubscriptionPeriod_ , msg);
//            scheduledSubscription = true;
//        }
//        else
//        {
//            scheduledSubscription = false;
//        }
//        return sent;
//    }
    return false;
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

void RNIService::removeSubscritions(inet::TCPSocket *socket)
{
    std::map<const inet::TCPSocket*, std::set<unsigned int>>::iterator it = socketToSubId_.find(socket);
    if(it != socketToSubId_.end())
    {
       std::set<unsigned int> subs = it->second;
       std::set<unsigned int>::iterator sit = it->second.begin();

       for(; sit != it->second.end(); ++sit)
       {
           delete subscriptions_[*sit];
       }
       socketToSubId_.erase(socket);
    }

}





