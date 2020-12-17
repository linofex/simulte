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

#ifndef __UEAPP_H
#define __UEAPP_H

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "common/LteCommon.h"
#include "inet/mobility/base/MovingMobilityBase.h"

/**
 * Accepts any number of incoming connections, and sends back whatever
 * arrives on them.
 */

enum Orientation {DX, SX} ;

class UeAppPacket;

class UeApp : public cSimpleModule, public inet::ILifecycle
{
  protected:


    inet::TCPSocket socket;
    inet::NodeStatus *nodeStatus = nullptr;
    inet::MovingMobilityBase *mobility_;


    // signals
    simsignal_t processingTimeSignal;
    simsignal_t propagationTimeSignal;

    simsignal_t deltaProcessingSignal;
    simsignal_t deltaPropagationSignal;



  protected:
    virtual bool isNodeUp();
    virtual void sendDown(cMessage *msg);
    virtual void startListening();
    virtual void stopListening();
    inet::Coord getCoord();
    Orientation getOrientation();
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;
    virtual bool handleOperationStage(inet::LifecycleOperation *operation, int stage, inet::IDoneCallback *doneCallback) override;

    // distance between when the message arrives and when the message is sent
    virtual double calculateDeltaPropagation(UeAppPacket * pkt);

    // distance between request and response
    virtual double calculateDeltaProcessing(UeAppPacket * pkt);

  public:
    UeApp() {}
    virtual ~UeApp(){}
};



#endif // ifndef __UEAPP_H

