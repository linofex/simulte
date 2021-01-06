//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

//
//  @author Angelo Buono
//

#ifndef __SIMULTE_MEWARNINGALERTAPP_H_
#define __SIMULTE_MEWARNINGALERTAPP_H_      virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;


#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

//MEWarningAlertPacket
#include "corenetwork/nodes/mec/MEPlatform/MEAppPacket_Types.h"
#include "../warningAlert/packets/WarningAlertPacket_m.h"

#include "apps/mec/MeApps/MeAppBase.h"

/**
 * See MEWarningAlertApp.ned
 */
class MEWarningAlertApp : public MeAppBase
{
    char* ueSimbolicAddress;
    char* meHostSimbolicAddress;
    inet::L3Address destAddress_;
    int size_;
    std::string subId;

    protected:

        virtual int numInitStages() const { return inet::NUM_INIT_STAGES; }
        void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
        void finish();

        void handleInfoUEWarningAlertApp(WarningAlertPacket* pkt);
        void handleInfoMEWarningAlertApp(WarningAlertPacket* pkt);

        void handleServicePacket();


        virtual void modifySubscription();

        virtual void handleSelfMsg(cMessage *msg);
        /* Utility functions */
       virtual void connect();
//        virtual void close();
//        virtual void sendPacket(cPacket *pkt);
//
//        /* TCPSocket::CallbackInterface callback methods */
       virtual void established(int connId);
       virtual void handleTcpMsg();


//        virtual void socketPeerClosed(int connId, void *yourPtr) override;
//        virtual void socketClosed(int connId, void *yourPtr) override;
//        virtual void socketFailure(int connId, void *yourPtr, int code) override;
//        virtual void socketStatusArrived(int connId, void *yourPtr, inet::TCPStatusInfo *status) override { delete status; }



};

#endif
