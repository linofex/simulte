/*
 * CircleNotificationSubscription.h
 *
 *  Created on: Dec 27, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_CIRCLENOTIFICATIONSUBSCRIPTION_H_
#define APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_CIRCLENOTIFICATIONSUBSCRIPTION_H_

#include "corenetwork/nodes/mec/MEPlatform/MeServices/Resources/SubscriptionBase.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include <set>
#include "corenetwork/nodes/mec/MEPlatform/MeServices/LocationService/resources/LocationApiDefs.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/LocationService/resources/TerminalLocation.h"

class LteBinder;
class CircleNotificationSubscription : public SubscriptionBase
{
    public:
        CircleNotificationSubscription();
        CircleNotificationSubscription(unsigned int subId, inet::TCPSocket *socket , const std::string& baseResLocation,  std::vector<cModule*>& eNodeBs);
        virtual ~CircleNotificationSubscription();

        nlohmann::ordered_json toJson() const override;
//
//
//
//        nlohmann::ordered_json toJsonCell(std::vector<MacCellId>& cellsID) const;
//        nlohmann::ordered_json toJsonUe(std::vector<MacNodeId>& uesID) const;
//        nlohmann::ordered_json toJson(std::vector<MacCellId>& cellsID, std::vector<MacNodeId>& uesID) const;


        virtual bool fromJson(const nlohmann::ordered_json& json);
        virtual void sendSubscriptionResponse();
        virtual void sendNotification();
        virtual void handleSubscription() override;


        bool findUe(MacNodeId nodeId);

    protected:

        LteBinder* binder; //used to retrieve NodeId - Ipv4Address mapping
        simtime_t lastNotification;
        bool firstNotificationSent;

        std::map<MacNodeId, bool> users; // optional: NO the bool is the initial potition wrt the area

        std::vector<TerminalLocation> terminalLocations; //it stores the user that entered or exited the are

        //callbackReference
        std::string callbackData;// optional: YES
        std::string notifyURL; // optional: NO


        std::string resourceURL;
        bool checkImmediate; // optional: NO
        std::string clientCorrelator; // optional: YES

        double frequency; // optional: NO


        inet::Coord center; // optional: NO, used for simulation

        double latitude; // optional: NO, used for simulation
        double longitude;// optional: NO, used for simulation


        double radius; // optional: NO

        int trackingAccuracy; // optional: NO
        LocationUtils::EnteringLeavingCriteria actionCriteria;// optional: NO

};



#endif /* APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_CIRCLENOTIFICATIONSUBSCRIPTION_H_ */
