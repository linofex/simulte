/*
 * L2MeasSubscription.h
 *
 *  Created on: Dec 19, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_MESERVICES_RNISERVICE_RESOURCES_L2MEASSUBSCRIPTION_H_
#define APPS_MEC_MESERVICES_RNISERVICE_RESOURCES_L2MEASSUBSCRIPTION_H_

#include "corenetwork/nodes/mec/MEPlatform/MeServices/Resources/SubscriptionBase.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/RNIService/resources/Ecgi.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/RNIService/resources/AssociateId.h"
#include "common/MecCommon.h"
class L2MeasSubscription : public SubscriptionBase
{

    typedef struct
    {
        std::string appIstanceId;
        AssociateId associteId_;
        Ecgi ecgi;
        Trigger trigger;
    }FilterCriteriaL2Meas;

    public:
        L2MeasSubscription();
        L2MeasSubscription(unsigned int subId, inet::TCPSocket *socket, const std::string& baseResLocation,  std::vector<cModule*>& eNodeBs);
        virtual ~L2MeasSubscription();
        virtual bool fromJson(const nlohmann::ordered_json& json);
        virtual void sendSubscriptionResponse();
        virtual void sendNotification();
        virtual void handleSubscription(){};

    protected:
        FilterCriteriaL2Meas filterCriteria_;

};




#endif /* APPS_MEC_MESERVICES_RNISERVICE_RESOURCES_L2MEASSUBSCRIPTION_H_ */
