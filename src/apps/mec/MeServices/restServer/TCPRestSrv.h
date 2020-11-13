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

#ifndef __INET_TCPRESTSRV_H
#define __INET_TCPRESTSRV_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"
#include <vector>
#include <map>

#include "resources/L2Meas.h"


#include "corenetwork/binder/LteBinder.h"


/**
 *
 *
 *
 */

class L2Meas;

class TCPRestSrv: public cSimpleModule, public inet::ILifecycle
{
  private:
    inet::TCPSocket serverSocket; // Used to listen incoming connections
    inet::TCPSocketMap socketMap; // Stores the connections
    std::map<std::string, std::string> usersLocations;
    UEWarningAlertApp_rest *app; //user
    LteBinder* binder_;
    cModule* meHost_;
    std::vector<cModule*> eNodeB_;     //eNodeB connected to the ME Host
    L2Meas L2MeasResource;

    void getConnectedEnodeB();


    // TODO data structure to save RNI info

  public:
    TCPRestSrv();
  protected:


    std::map<std::string, std::string> parseRequest(char *packet_);
    virtual void initialize(int stage) override;
    virtual int  numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;
    virtual void handleRequest(char *packet, inet::TCPSocket *socket);
    virtual void handleGetRequest(std::string& uri, inet::TCPSocket* socket);
    virtual void handlePostRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket);

    virtual ~TCPRestSrv();

    virtual bool handleOperationStage(inet::LifecycleOperation *operation, int stage, inet::IDoneCallback *doneCallback) override
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }
};


#endif // ifndef __INET_TCPRESTSRV_H

