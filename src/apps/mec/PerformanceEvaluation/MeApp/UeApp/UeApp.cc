#include "apps/mec/PerformanceEvaluation/MeApp/UeApp/UeApp.h"

#include "inet/common/RawPacket.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "apps/mec/PerformanceEvaluation/MeApp/packets/UeAppPacket.h"
#include "corenetwork/lteCellInfo/LteCellInfo.h"
#include "corenetwork/binder/LteBinder.h"
Define_Module(UeApp);

void UeApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {

        socket.setOutputGate(gate("tcpOut"));
        socket.setDataTransferMode(inet::TCP_TRANSFER_OBJECT);

        cModule *host = getContainingNode(this);
        mobility_ = check_and_cast<inet::MovingMobilityBase *>(host->getSubmodule("mobility"));

        nodeStatus = dynamic_cast<inet::NodeStatus *>(findContainingNode(this)->getSubmodule("status"));

        processingTimeSignal = registerSignal("processingTime");
        propagationTimeSignal = registerSignal("propagationTime");

        deltaProcessingSignal = registerSignal("deltaProcessing");
        deltaPropagationSignal = registerSignal("deltaPropagation");



    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        if (isNodeUp())
            startListening();
    }
}

bool UeApp::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == inet::NodeStatus::UP;
}

void UeApp::startListening()
{
    const char *localAddress = par("localAddress");
    int localPort = par("localPort");
    socket.renewSocket();
    socket.bind(localAddress[0] ? inet::L3Address(localAddress) : inet::L3Address(), localPort);
    socket.listen();
}

void UeApp::stopListening()
{
    socket.close();
}

void UeApp::sendDown(cMessage *msg)
{
    send(msg, "tcpOut");
}

void UeApp::handleMessage(cMessage *msg)
{
    if (!isNodeUp())
        throw cRuntimeError("Application is not running");
    if (msg->isSelfMessage()) {
        sendDown(msg);
    }
    else if (msg->getKind() == inet::TCP_I_PEER_CLOSED) {
        // we'll close too
        msg->setName("close");
        msg->setKind(inet::TCP_C_CLOSE);
        //stopListening();
        sendDown(msg);

        delete msg;
    }
    else if (msg->getKind() == inet::TCP_I_DATA || msg->getKind() == inet::TCP_I_URGENT_DATA) {

        UeAppPacket *pkt = check_and_cast<UeAppPacket *>(msg);
        EV <<"Received uePacket with seqNum: " << pkt->getSeqNum() << endl;
        inet::Coord reqCoords = pkt->getRequestCoord();
        inet::Coord resCoords = pkt->getResponseCoord();





        //emit stats

        emit(processingTimeSignal , (pkt->getResponseTime() - pkt->getRequestTime())*1000);
        emit(propagationTimeSignal , (msg->getArrivalTime() - pkt->getResponseTime())*1000);

        emit(deltaPropagationSignal, calculateDeltaPropagation(pkt)*100);
        emit(deltaProcessingSignal,  calculateDeltaProcessing(pkt)*100);

        EV<< "processing: " << (pkt->getResponseTime() - pkt->getRequestTime())*1000 << "ms" << endl;
        EV<< "propagation: " << (msg->getArrivalTime() - pkt->getResponseTime())*1000 << "ms" << endl;
        EV<< "min max: " << mobility_->getConstraintAreaMin().x << " " <<  mobility_->getConstraintAreaMax().x << endl;
        EV << "speed" <<  mobility_->getCurrentSpeed().x << endl;
        EV<< "distance propagation: " << calculateDeltaPropagation(pkt) << "m" << endl;
        EV<< "distance response: " << calculateDeltaProcessing(pkt) << "m" << endl;

        EV<< "distance: now " << getCoord().x << " on request " << reqCoords.x << endl;

        delete msg;
//        EV << pkt-;
//        EV << pkt;
    }
    else {
        // some indication -- ignore
        delete msg;
    }
}

void UeApp::refreshDisplay() const
{
}

bool UeApp::handleOperationStage(inet::LifecycleOperation *operation, int stage, inet::IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<inet::NodeStartOperation *>(operation)) {
        if ((inet::NodeStartOperation::Stage)stage == inet::NodeStartOperation::STAGE_APPLICATION_LAYER)
            startListening();
    }
    else if (dynamic_cast<inet::NodeShutdownOperation *>(operation)) {
        if ((inet::NodeShutdownOperation::Stage)stage == inet::NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            // TODO: wait until socket is closed
            stopListening();
    }
    else if (dynamic_cast<inet::NodeCrashOperation *>(operation))
        ;
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}


double UeApp::calculateDeltaPropagation(UeAppPacket * pkt)
{
    Orientation resOrientation = pkt->getResponseOrientation() > 0 ? DX : SX;
    Orientation currentOrientation = getOrientation();
    inet::Coord resCoord = pkt->getResponseCoord();
    inet::Coord currentCoord = getCoord();

    double distance = 0.;
    if(resOrientation != currentOrientation)
    {
        if(currentOrientation == DX)
        {
            distance += currentCoord.x - mobility_->getConstraintAreaMin().x;
            distance += resCoord.x - mobility_->getConstraintAreaMin().x;
        }
        else
        {
            distance += mobility_->getConstraintAreaMax().x - currentCoord.x;
            distance += mobility_->getConstraintAreaMax().x - resCoord.x;
        }

    }
    else
    {
        distance = currentCoord.distance(resCoord);
    }

    return distance;
}

double UeApp::calculateDeltaProcessing(UeAppPacket * pkt)
{
    Orientation resOrientation = pkt->getResponseOrientation() > 0 ? DX : SX;
    Orientation reqOrientation = pkt->getRequestOrientation() > 0 ? DX : SX;
    inet::Coord resCoord = pkt->getResponseCoord();
    inet::Coord reqCoord = pkt->getRequestCoord();

    double distance = 0.;
    if(resOrientation != reqOrientation)
    {
        if(reqOrientation == DX)
        {
            distance += reqCoord.x - mobility_->getConstraintAreaMin().x;
            distance += resCoord.x - mobility_->getConstraintAreaMin().x;
        }
        else
        {
            distance += mobility_->getConstraintAreaMax().x - reqCoord.x;
            distance += mobility_->getConstraintAreaMax().x - resCoord.x;
        }

    }
    else
    {
        distance = reqCoord.distance(resCoord);
    }

    return distance;

}


inet::Coord UeApp::getCoord()
{
    return mobility_->getCurrentPosition();

}

Orientation UeApp::getOrientation()
{
    return mobility_->getCurrentSpeed().x > 0 ? DX :SX ;

}

void UeApp::finish()
{

}


