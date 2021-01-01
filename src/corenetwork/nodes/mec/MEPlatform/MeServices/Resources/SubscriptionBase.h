/*
 * MeasRepUeNotification.h
 *
 *  Created on: Dec 10, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_MESERVICES_RESOURCES_SUBSCRIPTIONBASE_H_
#define APPS_MEC_MESERVICES_RESOURCES_SUBSCRIPTIONBASE_H_

#include "corenetwork/nodes/mec/MEPlatform/MeServices/Resources/AttributeBase.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/Resources/TimeStamp.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/httpUtils.h"

class LteCellInfo;

class SubscriptionBase : public AttributeBase
{
    public:
        SubscriptionBase();
        SubscriptionBase(unsigned int subId, inet::TCPSocket *socket , const std::string& baseResLocation,  std::vector<cModule*>& eNodeBs);
        virtual ~SubscriptionBase();

        nlohmann::ordered_json toJson() const override;

        void addEnodeB(std::vector<cModule*>& eNodeBs);
        void addEnodeB(cModule* eNodeB);

        nlohmann::ordered_json toJsonCell(std::vector<MacCellId>& cellsID) const;
        nlohmann::ordered_json toJsonUe(std::vector<MacNodeId>& uesID) const;
        nlohmann::ordered_json toJson(std::vector<MacCellId>& cellsID, std::vector<MacNodeId>& uesID) const;

        virtual void set_links(std::string& link);

        virtual bool fromJson(const nlohmann::ordered_json& json);
        virtual void sendSubscriptionResponse() = 0;
        virtual void sendNotification() = 0;


        virtual std::string getSubscriptionType() const;


    protected:
        inet::TCPSocket *socket_;
        TimeStamp timestamp_;

        std::string baseResLocation_;

        std::string clientHost_;
        std::string clientUri_;


        std::map<MacCellId, LteCellInfo*> eNodeBs_;
        unsigned int subscriptionId_;

        std::string subscriptionType_;
        std::string notificationType_;
        std::string links_;

        std::string callbackReference_;
        TimeStamp expiryTime_;
};







#endif /* APPS_MEC_MESERVICES_RESOURCES_SUBSCRIPTIONBASE_H_ */
