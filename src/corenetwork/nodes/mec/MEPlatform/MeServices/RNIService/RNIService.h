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

#ifndef _RNISERVICE_H
#define _RNISERVICE_H

#include "inet/common/INETDefs.h"
#include "common/MecCommon.h"

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"
#include <vector>
#include <map>

#include "../MeServiceBase/MeServiceBase.h"
#include "corenetwork/binder/LteBinder.h"
#include "resources/L2Meas.h"

#include "corenetwork/nodes/mec/MEPlatform/MeServices/RNIService/resources/L2MeasSubscription.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/RNIService/resources/MeasRepUeSubscription.h"

/**
 *
 *
 *
 */


class L2Meas;
class SubscriptionBase;

class RNIService: public MeServiceBase
{
  private:

    L2Meas L2MeasResource_;


    typedef std::map<unsigned int, L2MeasSubscription > L2MeasSubscriptions;
    L2MeasSubscriptions l2MeasSubscriptions_;

    typedef std::map<unsigned int, MeasRepUeSubscription > MeasRepUeSubscriptions;
    MeasRepUeSubscriptions measRepUeSubscriptions_;


    //change it from socket|* to connId or const socket* and move it to serviceBase
    std::map<const inet::TCPSocket*, std::set<unsigned int>> socketToSubId_;

    typedef std::map<unsigned int, SubscriptionBase*> Subscriptions;
    Subscriptions subscriptions_;

    bool scheduledSubscription;

  public:
    RNIService();
  protected:

    virtual void initialize(int stage) override;
    virtual void finish() override;

    virtual void handleGETRequest(const std::string& uri, inet::TCPSocket* socket) override;
    virtual void handlePOSTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket) override;
    virtual void handlePUTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket) override;
    virtual void handleDELETERequest(const std::string& uri, inet::TCPSocket* socket) override;
    virtual bool handleSubscriptionType(cMessage *msg);

    virtual ~RNIService();


};


#endif // ifndef _RNISERVICE_H

