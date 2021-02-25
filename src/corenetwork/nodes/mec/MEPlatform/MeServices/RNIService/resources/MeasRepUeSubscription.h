/*
 * L2MeasSubscription.h
 *
 *  Created on: Dec 19, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_MESERVICES_RNISERVICE_RESOURCES_MEASREPUESUBSCRIPTION_H_
#define APPS_MEC_MESERVICES_RNISERVICE_RESOURCES_MEASREPUESUBSCRIPTION_H_

#include "corenetwork/nodes/mec/MEPlatform/MeServices/Resources/SubscriptionBase.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/RNIService/resources/Ecgi.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/RNIService/resources/AssociateId.h"
#include "common/MecCommon.h"
class MeasRepUeSubscription : public SubscriptionBase
{

    typedef struct
    {
        std::string appIstanceId;
        AssociateId associteId_;
        Ecgi ecgi;
        Trigger trigger;
    }FilterCriteriaAssocTri;

    public:
        MeasRepUeSubscription();
        MeasRepUeSubscription(unsigned int subId, inet::TCPSocket *socket, const std::string& baseResLocation,  std::vector<cModule*>& eNodeBs);
        virtual ~MeasRepUeSubscription();
        virtual bool fromJson(const nlohmann::ordered_json& json);
        virtual void sendSubscriptionResponse();
        virtual void sendNotification();
        virtual void handleSubscription(){};
    protected:
        FilterCriteriaAssocTri filterCriteria_;

};




#endif /* APPS_MEC_MESERVICES_RNISERVICE_RESOURCES_MEASREPUESUBSCRIPTION_H_ */
