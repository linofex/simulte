/*
 * CircleNotificationSubscription.h
 *
 *  Created on: Dec 27, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_CIRCLENOTIFICATIONSUBSCRIPTION_H_
#define APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_CIRCLENOTIFICATIONSUBSCRIPTION_H_

#include "apps/mec/MeServices/Resources/SubscriptionBase.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include <set>
enum EnteringLeavingCriteria {Entering, Leaving};

class CircleNotificationSubscription : public SubscriptionBase
{
    public:
        CircleNotificationSubscription();
        CircleNotificationSubscription(unsigned int subId, inet::TCPSocket *socket , const std::string& baseResLocation,  std::vector<cModule*>& eNodeBs);
        virtual ~CircleNotificationSubscription();

//       nlohmann::ordered_json toJson() const override;
//
//
//
//        nlohmann::ordered_json toJsonCell(std::vector<MacCellId>& cellsID) const;
//        nlohmann::ordered_json toJsonUe(std::vector<MacNodeId>& uesID) const;
//        nlohmann::ordered_json toJson(std::vector<MacCellId>& cellsID, std::vector<MacNodeId>& uesID) const;


        virtual bool fromJson(const nlohmann::ordered_json& json);
        virtual void sendSubscriptionResponse();
        virtual void sendNotification();
        virtual void foo(){EV << "YESSSS" << endl;}


    protected:
        std::set<MacNodeId> uesId; // optional: NO
        std::set<inet::IPv4Address> uesAddress; // optional: NO

        //callbackReference
        std::string callbackData;// optional: YES
        std::string notifyURL; // optional: NO

        bool checkImmediate; // optional: NO
        std::string clientCorrelator; // optional: YES

        double frequency; // optional: NO


        inet::Coord center; // optional: NO, used for simulation

        double latitude; // optional: NO, used for simulation
        double longitude;// optional: NO, used for simulation


        double radius; // optional: NO

        int trackingAccuracy; // optional: NO
        EnteringLeavingCriteria actionCriteria;// optional: NO
};



#endif /* APPS_MEC_MESERVICES_LOCATIONSERVICE_RESOURCES_CIRCLENOTIFICATIONSUBSCRIPTION_H_ */
