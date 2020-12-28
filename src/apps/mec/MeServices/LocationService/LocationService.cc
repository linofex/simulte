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
#include "apps/mec/MeServices/LocationService/LocationService.h"

#include "apps/mec/MeServices/LocationService/resources/CircleNotificationSubscription.h"


#include <string>
#include <vector>
//#include "apps/mec/MeServices/packets/HttpResponsePacket.h"
#include "apps/mec/MeServices/httpUtils/httpUtils.h"
#include "common/utils/utils.h"
#include "apps/mec/MeServices/LocationService/packets/subscriptionTimer_m.h"



Define_Module(LocationService);


LocationService::LocationService(){
    baseUriQueries_ = "/example/location/v2/queries";
    baseUriSubscriptions_ = "/example/location/v2/subscriptions";

    baseSubscriptionLocation_ = host_+ baseUriSubscriptions_ + "/";
    subscriptionId_ = 0;
    subscriptions_.clear();
    supportedQueryParams_.insert("address");
    supportedQueryParams_.insert("latitude");
    supportedQueryParams_.insert("longitude");
    supportedQueryParams_.insert("zone");

    scheduledSubscription = false;
    // supportedQueryParams_s_.insert("ue_ipv6_address");
}

void LocationService::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        return;
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        MeServiceBase::initialize(stage);
        LocationResource_.addEnodeB(eNodeB_);
        LocationResource_.addBinder(binder_);
        LocationResource_.setBaseUri(host_+baseUriQueries_);
        LocationSubscriptionEvent_ = new cMessage("LocationSubscriptionEvent");
        LocationSubscriptionPeriod_ = par("LocationSubscriptionPeriod");

        cOvalFigure *circle = new cOvalFigure("circle");
        circle->setBounds(cFigure::Rectangle(200,200,60,60));
        circle->setLineWidth(2);
        circle->setLineStyle(cFigure::LINE_DOTTED);

        getSimulation()->getSystemModule()->getCanvas()->addFigure(circle);

    }
}

bool LocationService::manageSubscription()
{
    if(currentSubscriptionServed_->isName("subscriptionTimer"))
    {
        subscriptionTimer *subTimer = check_and_cast<subscriptionTimer*>(currentSubscriptionServed_);
        if(subscriptions_.find(subTimer->getSubId()) != subscriptions_.end())
        {
            SubscriptionBase * sub = subscriptions_[subTimer->getSubId()]; //upcasting (getSubscriptionType is in Subscriptionbase)
            if(sub->getSubscriptionType().compare("circleNotificationSubscription") == 0)
            {
                EV << "LocationService::handleMessage - handling subId: " << subTimer->getSubId() << endl;
                CircleNotificationSubscription *cir = (CircleNotificationSubscription* ) subscriptions_[subTimer->getSubId()];
                cir->handleSubscription();
                if(currentSubscriptionServed_!= nullptr)
                        delete currentSubscriptionServed_;
                currentSubscriptionServed_ = nullptr;
                return true;

            }
        }
        else{
            EV << "NO" << endl;
            if(currentSubscriptionServed_!= nullptr)
                    delete currentSubscriptionServed_;
            currentSubscriptionServed_ = nullptr;
            return true;
        }
    }
}

void LocationService::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
    {
        if(msg->isName("subscriptionTimer"))
        {
            subscriptionTimer *subTimer = check_and_cast<subscriptionTimer*>(msg);
            subscriptionTimer *subTimerDup = subTimer->dup();
            newSubscriptionEvent(subTimerDup);
            scheduleAt(simTime()+subTimer->getPeriod(), msg);
            return;
        }

    }
        MeServiceBase::handleMessage(msg);
}

void LocationService::handleGETRequest(const std::string& uri, inet::TCPSocket* socket)
{
    EV_INFO << "handleGETRequest" << endl;
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
        if(resourceType.compare("users") == 0 )
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
                std::vector<IPv4Address> ues;

                typedef std::map<std::string, std::vector<std::string>> queryMap;
                queryMap queryParamsMap; // e.g cell_id -> [0, 1]

                std::vector<std::string>::iterator it  = queryParameters.begin();
                std::vector<std::string>::iterator end = queryParameters.end();
                std::vector<std::string> params;
                std::vector<std::string> splittedParams;
                for(; it != end; ++it){
                    if(it->rfind("accessPointId", 0) == 0) // cell_id=par1,par2
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
                    else if(it->rfind("address", 0) == 0)
                    {

                        //NON VA BENE!
                        params = lte::utils::splitString(*it, "=acr:");
                        if(params.size()!= 2) //must be param=acr:values
                        {
                            Http::send400Response(socket);
                            return;
                        }
                        splittedParams = lte::utils::splitString(params[1], ",");
                        std::vector<std::string>::iterator pit  = splittedParams.begin();
                        std::vector<std::string>::iterator pend = splittedParams.end();
                        for(; pit != pend; ++pit){
                           //manage ipv4 address without any macnode id
                            //or do the conversion inside Location..
                           ues.push_back(IPv4Address((*pit).c_str()));
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
                    Http::send200Response(socket, LocationResource_.toJson(cellIds, ues).dump(2).c_str());
                }
                else if(ues.empty() && !cellIds.empty())
                {
                    Http::send200Response(socket, LocationResource_.toJsonCell(cellIds).dump(2).c_str());
                }
                else if(!ues.empty() && cellIds.empty())
               {
                   Http::send200Response(socket, LocationResource_.toJsonUe(ues).dump(2).c_str());
               }
               else
               {
                   Http::send400Response(socket);
               }

            }
            else if (splittedUri.size() == 1 ){ //no query params
                Http::send200Response(socket,LocationResource_.toJson().dump(2).c_str());
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

void LocationService::handlePOSTRequest(const std::string& uri,const std::string& body, inet::TCPSocket* socket)
{
    EV << "LocationService::handlePOSTRequest" << endl;
    // uri must be in form example/location/v2/subscriptions/sub_type
    // or
    // example/location/v2/subscriptions/type/sub_type
    std::size_t lastPart = uri.find_last_of("/");
    if(lastPart == std::string::npos)
    {
        EV << "1" << endl;
        Http::send404Response(socket); //it is not a correct uri
        return;
    }

    // find_last_of does not take in to account if the uri has a last /
    // in this case subscriptionType would be empty and the baseUri == uri
    // by the way the next if statement solve this problem
    std::string baseUri = uri.substr(0,lastPart);
    std::string subscriptionType =  uri.substr(lastPart+1);

    EV << "baseuri: "<< baseUri << endl;

    // it has to be managed the case when the sub is /area/circle (it has two slashes)
    if(baseUri.compare(baseUriSubscriptions_+"/area") == 0)
    {
        EV << "subscriptionType: "<< subscriptionType << endl;

        if(subscriptionType.compare("circle") == 0)
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

            CircleNotificationSubscription* newSubscription  = new CircleNotificationSubscription(subscriptionId_, socket , baseSubscriptionLocation_,  eNodeB_);
            bool res = newSubscription->fromJson(jsonBody);
            //correct subscription post
            if(res)
            {
                subscriptions_[subscriptionId_] = newSubscription;
                //start timer
                subscriptionTimer* msg = new subscriptionTimer("subscriptionTimer");
                newSubscription->setNotificationTrigger(msg);
                msg->setSubId(subscriptionId_);
                msg->setPeriod(0.5);
                scheduleAt(simTime() + msg->getPeriod(), msg);
                subscriptionId_ ++;
                //circleSubscriptions_[subscriptionId_] = newSubscription;
                //socketToSubId_[socket].insert(subscriptionId_);
            }

        }
        else
        {
            Http::send404Response(socket); //resource not found
            return;
        }
    }

    else if(baseUri.compare(baseUriSubscriptions_) == 0)
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
        //  - LocationurementNotification

        SubscriptionInfo newSubscription;

        if(subscriptionType.compare("L2_meas") == 0)
        {

        }
    }
    else
    {
        Http::send404Response(socket); //resource not found
    }
}

void LocationService::handlePUTRequest(const std::string& uri,const std::string& body, inet::TCPSocket* socket){}

void LocationService::handleDELETERequest(const std::string& uri, inet::TCPSocket* socket)
{
//    DELETE /exampleAPI/location/v1/subscriptions/area/circle/sub123 HTTP/1.1
//    Accept: application/xml
//    Host: example.com

    EV << "LocationService::handleDELETERequest" << endl;
    // uri must be in form example/location/v2/subscriptions/sub_type
    // or
    // example/location/v2/subscriptions/type/sub_type
    std::size_t lastPart = uri.find_last_of("/");
    if(lastPart == std::string::npos)
    {
        EV << "1" << endl;
        Http::send404Response(socket); //it is not a correct uri
        return;
    }

    // find_last_of does not take in to account if the uri has a last /
    // in this case subscriptionType would be empty and the baseUri == uri
    // by the way the next if statement solve this problem
    std::string baseUri = uri.substr(0,lastPart);
    std::string ssubId =  uri.substr(lastPart+1);

    EV << "baseuri: "<< baseUri << endl;

    // it has to be managed the case when the sub is /area/circle (it has two slashes)
    if(baseUri.compare(baseUriSubscriptions_+"/area/circle") == 0)
    {
        int subId = std::stoi(ssubId);
        Subscriptions::iterator it = subscriptions_.find(subId);
        if(it != subscriptions_.end())
        {
            CircleNotificationSubscription *sub = (CircleNotificationSubscription*) it->second;
            subscriptionTimer* msg = sub->getNotificationTrigger();
            if(msg->isScheduled())
                cancelAndDelete(msg);
            delete it->second;
            subscriptions_.erase(it);

            Http::send204Response(socket);
        }
        else
        {
            Http::send400Response(socket);
        }
    }
    else
    {
        Http::send404Response(socket);
    }

}

bool LocationService::handleSubscriptionType(cMessage *msg)
{
    if(msg->getKind() == L2_MEAS_PERIODICAL)
    {
        return manageLocationSubscriptions(L2_MEAS_PERIODICAL);
    }
    delete msg;
     return false;
}

bool LocationService::manageLocationSubscriptions(Trigger trigger){
//    SubscriptionsStructure::iterator it = subscriptions_.find("L2_meas");
//    if(it != subscriptions_.end())
//    {
//        std::map<std::string, SubscriptionInfo>::iterator sit = it->second.begin();
//        while(sit != it->second.end())
//        {
//            std::string body;
//            //send response
//            if(trigger == L2_MEAS_PERIODICAL)
//            {
//                if(!sit->second.ues.empty() && !sit->second.cellIds.empty())
//                {
//                    body = LocationResource_.toJson(sit->second.cellIds, sit->second.ues).dump(2);
//                    std::cout << "ue e cell"<<std::endl;
//                }
//                else if(sit->second.ues.empty() && !sit->second.cellIds.empty())
//                {
//                    body = LocationResource_.toJsonCell(sit->second.cellIds).dump(2);
//                    std::cout << "cell"<<std::endl;
//                }
//                else if(!sit->second.ues.empty() && sit->second.cellIds.empty())
//                {
//                    body = LocationResource_.toJsonUe(sit->second.ues).dump(2);
//                    std::cout << "ue"<<std::endl;
//                }
//                else if(sit->second.ues.empty() && sit->second.cellIds.empty())
//                {
//                    body = LocationResource_.toJson().dump(2);
//                    std::cout << "tutti"<<std::endl;
//                }
//            }
//            else if(trigger == L2_MEAS_UTI_80)
//            {
//            }
//            if(sit->second.socket->getState() == TCPSocket::CONNECTED)
//            {
//                if(body.size() > 0)
//                {
//                    int slash = sit->second.consumerUri.find('/');
//                    std::string host = sit->second.consumerUri.substr(0, slash);
//                    std::string uri = sit->second.consumerUri.substr(slash);
//                    Http::sendPostRequest(sit->second.socket, body.c_str(), host.c_str(), uri.c_str());
//                }
//            sit++;
//            }
//            else
//            { //remove the subscription since the socket is not connected
//                it->second.erase(sit++);
//            }
//
//        }
//        if(!it->second.empty())
//        {
//            cMessage *msg = new cMessage("subscriptionEvent", L2_MEAS_PERIODICAL);
//            scheduleAt(simTime() + LocationSubscriptionPeriod_ , msg);
//            scheduledSubscription = true;
//        }
//        else
//        {
//            scheduledSubscription = false;
//        }
//    }
}

void LocationService::finish()
{
// TODO
    return;
}


void LocationService::refreshDisplay() const
{
// TODO
    return;
}


LocationService::~LocationService(){
    cancelAndDelete(LocationSubscriptionEvent_);
return;
}




