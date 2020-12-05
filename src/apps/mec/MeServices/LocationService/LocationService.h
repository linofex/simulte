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

#ifndef _LOCATIONSERVICE_H
#define _LOCATIONSERVICE_H

#include "inet/common/INETDefs.h"
#include "common/MecCommon.h"

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"
#include <vector>
#include <map>

#include "apps/mec/MeServices/GenericService/GenericService.h"
#include "corenetwork/binder/LteBinder.h"
#include "apps/mec/MeServices/LocationService/resources/LocationResource.h"


/**
 *
 *
 *
 */

typedef struct{
    inet::TCPSocket *socket;
    std::vector<MacNodeId> cellIds;
    std::vector<MacNodeId> ues;
    std::vector<Trigger> trigger;
    std::string consumerUri;
    std::string appInstanceId;
    std::string subscriptionType;
    std::string subscriptionId;

    double expiretaionTime;
} SubscriptionInfo;



class Location;

class LocationService: public GenericService
{
  private:

    LocationResource LocationResource_;
    typedef std::map<std::string, std::map<std::string, SubscriptionInfo >> SubscriptionsStructure;
    SubscriptionsStructure subscriptions_;

    double LocationSubscriptionPeriod_;
    cMessage *LocationSubscriptionEvent_;

    unsigned int subscriptionId_;
    std::string baseUriQueries_;
    std::string baseUriSubscriptions_;
    std::set<std::string>supportedQueryParams_;
    std::set<std::string>supportedSubscriptionParams_;
    
    bool scheduledSubscription;

    void manageLocationSubscriptions(Trigger trigger);

  public:
    LocationService();

  protected:

    virtual void initialize(int stage) override;
    virtual int  numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void handleGETRequest(const std::string& uri, inet::TCPSocket* socket);
    virtual void handlePOSTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket);
    virtual void handlePUTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket);
    virtual void handleDELETERequest(const std::string& uri, inet::TCPSocket* socket);
    virtual void handleSubscriptionType(cMessage *msg);


    virtual ~LocationService();


};


#endif // ifndef _LOCATIONSERVICE_H

