/*
 * CircleNotificationSubscription.cc
 *
 *  Created on: Dec 27, 2020
 *      Author: linofex
 */

#include "apps/mec/MeServices/LocationService/resources/CircleNotificationSubscription.h"


CircleNotificationSubscription::CircleNotificationSubscription(unsigned int subId, inet::TCPSocket *socket , const std::string& baseResLocation,  std::vector<cModule*>& eNodeBs):
SubscriptionBase(subId,socket,baseResLocation, eNodeBs){
    baseResLocation_+= "area/circle";

};

CircleNotificationSubscription::~CircleNotificationSubscription(){}


void CircleNotificationSubscription::sendSubscriptionResponse(){}
void CircleNotificationSubscription::sendNotification(){}



bool CircleNotificationSubscription::fromJson(const nlohmann::ordered_json& body)
{
    // ues; // optional: NO
    //
    ////callbackReference
    // callbackData;// optional: YES
    // notifyURL; // optional: NO
    //
    // checkImmediate; // optional: NO
    // clientCorrelator; // optional: YES
    //
    // frequency; // optional: NO
    // center; // optional: NO
    // radius; // optional: NO
    // trackingAccuracy; // optional: NO
    // EnteringLeavingCriteria; // optional: NO

    if(body.contains("circleNotificationSubscription")) // mandatory attribute
    {

        subscriptionType_ = "circleNotificationSubscription";
    }
    else
    {
        EV << "1" << endl;
        Http::send400Response(socket_);
        return false;
    }
    nlohmann::ordered_json jsonBody = body["circleNotificationSubscription"];

    if(jsonBody.contains("callbackReference"))
    {
        nlohmann::ordered_json callbackReference = jsonBody["callbackReference"];
        if(callbackReference.contains("callbackData"))
            callbackData = callbackReference["callbackData"];
        if(callbackReference.contains("notifyURL"))
            notifyURL = callbackReference["notifyURL"];
        else
        {
            EV << "2" << endl;

            Http::send400Response(socket_); //notifyUrl is mandatory
            return false;
        }

    }
    else
    {
        EV << "3" << endl;

        Http::send400Response(socket_); // callbackReference is mandatory and takes exactly 1 att
        return false;
    }

    if(jsonBody.contains("checkImmediate"))
    {
        std::string check = jsonBody["checkImmediate"];
        checkImmediate = check.compare("true") == 0 ? true : false;
    }
    else
    {
        checkImmediate = false;
    }

    if(jsonBody.contains("clientCorrelator"))
    {
       clientCorrelator = jsonBody["clientCorrelator"];
    }

    if(jsonBody.contains("frequency"))
    {
       frequency = jsonBody["frequency"];
    }
    else
   {
        EV << "4" << endl;

       Http::send400Response(socket_); //frequency is mandatory
       return false;
   }

    if(jsonBody.contains("radius"))
    {
        radius = jsonBody["radius"];
    }
    else
   {
        EV << "5" << endl;

       Http::send400Response(socket_); //radius is mandatory
       return false;
   }

    if(jsonBody.contains("coords") || (jsonBody.contains("latitude") && jsonBody.contains("longitude")))
    {
        if(jsonBody.contains("coords"))
        {
            center.x = jsonBody["coords"]["x"];
            center.y = jsonBody["coords"]["y"];
            center.z = jsonBody["coords"]["z"];
        }

        else
        {
            latitude = jsonBody["latitude"];
            longitude = jsonBody["longitude"];
        }

    }
    else
   {
        EV << "6" << endl;

       Http::send400Response(socket_); //postion is mandatory
       return false;
   }

    if(jsonBody.contains("trackingAccuracy"))
    {
        trackingAccuracy = jsonBody["trackingAccuracy"];
    }
    else
   {
        EV << "7" << endl;

       Http::send400Response(socket_); //trackingAccuracy is mandatory
       return false;
   }

    if(jsonBody.contains("enteringLeavingCriteria"))
    {
        std::string criteria = jsonBody["enteringLeavingCriteria"];
        if(criteria.compare("Entering") == 0)
        {
            actionCriteria = Entering;
        }
        else if(criteria.compare("Leaving") == 0)
        {
            actionCriteria = Leaving;
        }

    }
    else
   {
        EV << "8" << endl;

       Http::send400Response(socket_); //trackingAccuracy is mandatory
       return false;
   }

return true;
}
