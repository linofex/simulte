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

#include "UEWarningAlertApp_rest.h"

#include "inet/common/RawPacket.h"
#include "apps/mec/restServer/utils/utils.h"
#include "string.h"

Define_Module(UEWarningAlertApp_rest);

UEWarningAlertApp_rest::UEWarningAlertApp_rest(){
    selfStart_ = NULL;
    selfSender_ = NULL;
    selfStop_ = NULL;
}

UEWarningAlertApp_rest::~UEWarningAlertApp_rest(){
    cancelAndDelete(selfStart_);
    cancelAndDelete(selfSender_);
    cancelAndDelete(selfStop_);
}

void UEWarningAlertApp_rest::initialize(int stage)
{
    EV << "UEWarningAlertApp_rest::initialize - stage " << stage << endl;
    cSimpleModule::initialize(stage);
    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    //retrieve parameters
    size_ = par("packetSize");
    period_ = par("period");
    localPort_ = par("localPort");
    destPort_ = par("destPort");
    sourceSimbolicAddress = (char*)getParentModule()->getFullName();
    destSimbolicAddress = (char*)par("destAddress").stringValue();
    destAddress_ = inet::L3AddressResolver().resolve(destSimbolicAddress);
    requiredRam = par("requiredRam");
    requiredDisk = par("requiredDisk");
    requiredCpu = par("requiredCpu").doubleValue();

    //binding socket UDP
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort_);

    //retrieving car cModule
    ue = this->getParentModule();

    //retrieving mobility module
    cModule *temp = getParentModule()->getSubmodule("mobility");
    if(temp != NULL){
        mobility = check_and_cast<inet::IMobility*>(temp);
    }
    else {
        EV << "UEWarningAlertApp_rest::initialize - \tWARNING: Mobility module NOT FOUND!" << endl;
        throw cRuntimeError("UEWarningAlertApp_rest::initialize - \tWARNING: Mobility module NOT FOUND!");
    }

    stopTime = par("stopTime");
    //initializing the auto-scheduling messages
    selfSender_ = new cMessage("selfSender");
    selfStart_ = new cMessage("selfStart");
    selfStop_ = new cMessage("selfStop");

    //starting UEWarningAlertApp_rest
    simtime_t startTime = par("startTime");
    EV << "UEWarningAlertApp_rest::initialize - starting sendStartMEWarningAlertApp() in " << startTime << " seconds " << endl;
    scheduleAt(simTime() + startTime, selfStart_);

    //testing
    EV << "UEWarningAlertApp_rest::initialize - sourceAddress: " << sourceSimbolicAddress << " [" << inet::L3AddressResolver().resolve(sourceSimbolicAddress).str()  <<"]"<< endl;
    EV << "UEWarningAlertApp_rest::initialize - destAddress: " << destSimbolicAddress << " [" << destAddress_.str()  <<"]"<< endl;
    EV << "UEWarningAlertApp_rest::initialize - binding to port: local:" << localPort_ << " , dest:" << destPort_ << endl;
}

void UEWarningAlertApp_rest::handleMessage(cMessage *msg)
{
    EV << "UEWarningAlertApp_rest::handleMessage" << endl;
    // Sender Side
    if (msg->isSelfMessage())
    {
       EV << "####" <<msg->getName() << endl;
       if(!strcmp(msg->getName(), "selfStart"))   sendStartUEWarningAlertApp();
       else if(!strcmp(msg->getName(), "selfStop")) sendStopUEWarningAlertApp();
       else    throw cRuntimeError("UEWarningAlertApp_rest::handleMessage - \tWARNING: Unrecognized self message");
    }
    // Receiver Side
    else{
        EV << "###RICEVUTO: " << msg;
        if(msg->getKind() == UDP_I_DATA){
            handleUDPPacket(utils::getPacketPayload(msg));
        }
        delete msg;
    }
}


void UEWarningAlertApp_rest::handleUDPPacket(char* payload){
    // ACK from app
    if(strstr(payload, "OK START APP") != nullptr){
        // stop sending START APP
        if(selfStart_->isScheduled()){
            cancelEvent(selfStart_);
        }
        EV << "##OK START APP\n";
        //start STOP APP timer
        scheduleAt(simTime() + stopTime, selfStop_);
    }
    else if(strstr(payload, "OK STOP APP") != nullptr){
        //do nothing
    }
    // alert message
    else if(strstr(payload, "ALERT MESSAGE") != nullptr){
        EV << payload << endl;
    }

    // Bad UDP message
    else{
        throw cRuntimeError("UEWarningAlertApp_rest::handleMessage - \tWARNING: Unrecognized UDP message");
    }
}


void UEWarningAlertApp_rest::finish()
{
    // ensuring there is no selfStop_ scheduled!
    if(selfStop_->isScheduled())
        cancelEvent(selfStop_);
}

inet::Coord UEWarningAlertApp_rest::getPosition(){
    return mobility->getCurrentPosition();
}

void UEWarningAlertApp_rest::sendStartUEWarningAlertApp()
{
    EV << "###UEWarningAlertApp_rest::sendStartUEWarningAlertApp - Sending\n";
    std::ostringstream payload;
    payload << "START APP\nUser: " << getFullName();

    RawPacket *pck  = utils::createUDPPacket(payload.str());
    socket.sendTo(pck, destAddress_, destPort_);

    //rescheduling
    if(selfStart_->isScheduled()) cancelEvent(selfStart_);
    scheduleAt(simTime() + period_ , selfStart_);
}

void UEWarningAlertApp_rest::sendStopUEWarningAlertApp(){
    EV << "###UEWarningAlertApp_rest::sendStopUEWarningAlertApp - Sending\n";
    std::ostringstream payload;
    payload << "STOP APP\nUser: " << getFullName();

    RawPacket *pck  = utils::createUDPPacket(payload.str());
    socket.sendTo(pck, destAddress_, destPort_);

}
