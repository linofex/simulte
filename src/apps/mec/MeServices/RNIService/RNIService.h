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

#ifndef __INET_RNISERVICE_H
#define __INET_RNISERVICE_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"
#include <vector>
#include <map>

#include "apps/mec/MeServices/GenericService/GenericService.h"
#include "corenetwork/binder/LteBinder.h"
#include "resources/L2Meas.h"


/**
 *
 *
 *
 */

class L2Meas;

class RNIService: public GenericService
{
  private:

    L2Meas L2MeasResource_;
    std::string baseUriQueries_;
    std::string baseUriSubscriptions_;
    std::set<std::string>supportedQueryParams_;
    std::set<std::string>supportedSubscriptionParams_;
    
  public:
    RNIService();
  protected:

//    std::map<std::string, std::string> parseRequest(char *packet_);
    virtual void initialize(int stage) override;
    virtual int  numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;
//    virtual void handleRequest(char *packet, inet::TCPSocket *socket);

    virtual void handleGETRequest(const std::string& uri, inet::TCPSocket* socket);
    virtual void handlePOSTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket);
    virtual void handlePUTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket);
    virtual void handleDELETERequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket);

    virtual ~RNIService();

    virtual bool handleOperationStage(inet::LifecycleOperation *operation, int stage, inet::IDoneCallback *doneCallback) override
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }
};


#endif // ifndef __INET_RNISERVICE_H

