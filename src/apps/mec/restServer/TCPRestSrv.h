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

namespace inet {

/**
 *
 *
 *
 */
class INET_API TCPRestSrv : public cSimpleModule, public ILifecycle
{
  protected:
    TCPSocket serverSocket; // Used to listen incoming connections
    TCPSocketMap socketMap; // Stores the connections
    // TODO data structure to save RNI info

  protected:

    std::map<std::string, std::string> parseRequest(char *packet_);
    virtual void sendResponse(cMessage *msg, TCPSocket *socket);
    virtual void initialize(int stage) override;
    virtual int  numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;
    virtual void handleRequest(char *packet, TCPSocket *socket);
    virtual void handleGetRequest(std::string, TCPSocket *socket);

    virtual ~TCPRestSrv();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }
};

} // namespace inet

#endif // ifndef __INET_TCPRESTSRV_H

