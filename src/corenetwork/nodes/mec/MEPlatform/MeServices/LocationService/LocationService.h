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

#include "corenetwork/nodes/mec/MEPlatform/MeServices/LocationService/resources/LocationResource.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/MeServiceBase/MeServiceBase.h"
#include "corenetwork/binder/LteBinder.h"


/**
 *
 *
 *
 */


class Location;

class LocationService: public MeServiceBase
{
  private:

    LocationResource LocationResource_;

    double LocationSubscriptionPeriod_;
    cMessage *LocationSubscriptionEvent_;
    
    bool scheduledSubscription;

    bool manageLocationSubscriptions(Trigger trigger);

  public:
    LocationService();

  protected:

    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void handleGETRequest(const std::string& uri, inet::TCPSocket* socket);
    virtual void handlePOSTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket);
    virtual void handlePUTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket);
    virtual void handleDELETERequest(const std::string& uri, inet::TCPSocket* socket);
    virtual bool handleSubscriptionType(cMessage *msg);

    /*
     * This method is called for every element in the subscriptions_ queue.
     */
    virtual bool manageSubscription();
    virtual ~LocationService();


};


#endif // ifndef _LOCATIONSERVICE_H

