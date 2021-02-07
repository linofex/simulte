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

#include "inet/networklayer/common/L3AddressResolver.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/MeServiceBase/SocketManager.h"
#include "inet/common/INETDefs.h"
#include "inet/common/INETUtils.h"

#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/httpUtils.h"
#include "inet/common/RawPacket.h"
#include "common/utils/utils.h"
#include <vector>
#include <map>
#include "MEClusterizeService.h"


Define_Module(MEClusterizeService);

MEClusterizeService::MEClusterizeService(){

    colors.push_back("black");
    colors.push_back("grey");
    colors.push_back("brown");
    colors.push_back("purple");
    colors.push_back("magenta");
    colors.push_back("red");
    colors.push_back("orange");
    colors.push_back("yellow");
    colors.push_back("green");
    colors.push_back("olive");
    colors.push_back("cyan");
    colors.push_back("blue");
    colors.push_back("navy");
    colors.push_back("violet");
    eventID = 0;

    responseMessage = "";
    responseMessageLength = 0;
    receivingMessage = false;
}

MEClusterizeService::~MEClusterizeService(){}

void MEClusterizeService::initialize(int stage)
{
    EV << "MEClusterizeService::initialize - stage " << stage << endl;
    cSimpleModule::initialize(stage);
    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;
    //----------------------------------
    //auto-scheduling
    selfSender_ = new cMessage("selfSender");
    period_ = par("period");

    errorSpeed.setName("speed error");
    //----------------------------------
    //parent modules
    mePlatform = getParentModule();
    if(mePlatform != NULL){
        meHost = mePlatform->getParentModule();
        //configuring gate sizes
        int maxMEApps = 0;
        if(meHost->hasPar("maxMEApps"))
                maxMEApps = meHost->par("maxMEApps").longValue();
        else
            throw cRuntimeError("MEClusterizeService::initialize - \tFATAL! Error when getting meHost.maxMEApps parameter!");
        this->setGateSize("meAppOut", maxMEApps);
        this->setGateSize("meAppIn", maxMEApps);
    }
    else{
        EV << "MEClusterizeService::initialize - ERROR getting mePlatform cModule!" << endl;
        throw cRuntimeError("MEClusterizeService::initialize - \tFATAL! Error when getting getParentModule()");
    }
    //----------------------------------

    //Location service
//    const char *localAddress = par("localAddress");
//    int localPort = par("localPort");


    socket.readDataTransferModePar(*this);
//    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);

    //socket.setCallbackObject(this);
    socket.setOutputGate(gate("tcpOut"));

    socket.setCallbackObject(this);
    sendTimer = new cMessage("send");

    selfGet_ = new cMessage("GetRequest");
    getPeriod_ = par("getPeriod");

    respTime = registerSignal("respTime");
    //txMode information for sending INFO_MEAPP
    preconfiguredTxMode = par("preconfiguredTxMode").stringValue();
    //----------------------------------
    //binder
    binder_ = getBinder();
    //----------------------------------
    //statistics
    stat_ = check_and_cast<MultihopD2DStatistics*>(getModuleByPath("d2dMultihopStatistics"));
    //--------------------------------------
    //starting MEClusterizeService
    simtime_t startTime = par("startTime");
    scheduleAt(simTime() + startTime, selfSender_);
    EV << "MEClusterizeService::initialize - \t starting compute() in " << startTime << " seconds " << endl;
}

void MEClusterizeService::connect()
{
    // we need a new connId if this is not the first connection
    socket.renewSocket();
    // connect
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");
    inet::L3Address destination;
    inet::L3AddressResolver().tryResolve(connectAddress, destination);
    if (destination.isUnspecified()) {
        EV_ERROR << "Connecting to " << connectAddress << " port=" << connectPort << ": cannot resolve destination address\n";
    }
    else {
        EV_INFO << "Connecting to " << connectAddress << "(" << destination << ") port=" << connectPort << endl;
        socket.connect(destination, connectPort);
    }
}

void MEClusterizeService::requestUserPositions()
{
    std::string body = "";
    std::string parameters = getCarsAddressesParameters();
    std::string uri;
    if(!parameters.empty())
        uri = "/example/location/v2/queries/users?address="+parameters;
    else
        uri = "/example/location/v2/queries/users";
    std::string host = socket.getRemoteAddress().str()+":"+std::to_string(socket.getRemotePort());
    Http::sendGetRequest(&socket, body.c_str(), host.c_str(), uri.c_str());
    t1 = simTime();
}

/*
 * #################################################################################################################################
 */
void MEClusterizeService::handleMessage(cMessage *msg)
{
    EV << "MEClusterizeService::handleMessage" << endl;
    if (msg->isSelfMessage()){
        if(msg->isName("GetRequest"))
        {
            requestUserPositions();
            return;
        }
        compute();
        sendConfig();
        scheduleAt(simTime() + period_, selfSender_);
    }
    else
    {
        if(msg->getKind() == inet::TCP_I_DATA || msg->getKind() == inet::TCP_I_ESTABLISHED ||
           msg->getKind() == inet::TCP_I_URGENT_DATA || msg->getKind() == inet::TCP_I_CLOSED ||
           msg->getKind() == inet::TCP_I_PEER_CLOSED || msg->getKind() == inet::TCP_I_CONNECTION_RESET ||
           msg->getKind() == inet::TCP_I_CONNECTION_REFUSED )
        {
            socket.processMessage(msg);
            return;
        }
//
        ClusterizePacket* pkt = check_and_cast<ClusterizePacket*>(msg);
        if (pkt == 0)
            throw cRuntimeError("MEClusterizeService::handleMessage - \tFATAL! Error when casting to ClusterizePacket");

        if(!strcmp(pkt->getType(), INFO_UEAPP))
        {
            ClusterizeInfoPacket* ipkt = check_and_cast<ClusterizeInfoPacket*>(msg);
            handleClusterizeInfo(ipkt);
        }

        if(!strcmp(pkt->getType(), ADD_CAR))
        {
            handleClusterizeNewCar(pkt);
        }

        else if(!strcmp(pkt->getType(), STOP_MEAPP))     handleClusterizeStop(pkt);

       delete pkt;
    }
}

/*
 * ################################################################################################################################################
 */
void MEClusterizeService::sendConfig()
{
    EV << "MEClusterizeService::sendConfig - start" << endl;
    if(cars.empty())
        return;

    std::map<int, cluster>::iterator cl_it;
    //DOWNLINK_UNICAST_TX_MODE
    if (!strcmp(preconfiguredTxMode.c_str(), DOWNLINK_UNICAST_TX_MODE))
    {
        EV << "MEClusterizeService::sendConfig - DOWNLINK_UNICAST_TX_MODE" << endl;

        for(cl_it = clusters.begin(); cl_it != clusters.end(); cl_it++)
        {
            for(int memberKey : cl_it->second.members)
            {
                ClusterizeConfigPacket* pkt = ClusterizePacketBuilder().buildClusterizeConfigPacket(0, 0, eventID, 1, 0, 0, "", "", cars[memberKey].clusterID, cl_it->second.color.c_str(), cars[memberKey].txMode.c_str(), cars[memberKey].following.c_str(), cars[memberKey].follower.c_str(),cl_it->second.membersList.c_str(), cl_it->second.accelerations);
                //sending to the member MEClusterizeApp (on the corresponded gate!)

                //sending distanceGaps
                int size = cl_it->second.distancies.size();
                pkt->setDistanciesArraySize(size);
                for(int i=0; i < size; i++)     pkt->setDistancies(i, cl_it->second.distancies.at(i));

                send(pkt, "meAppOut", memberKey);
                //testing
                EV << "\nMEClusterizeService::sendConfig - sending ClusterizeConfig to CM: "  << cars[memberKey].symbolicAddress << " (txMode: INFRASTRUCTURE_UNICAST_TX_MODE) " << endl;
                EV << "MEClusterizeService::sendConfig - \t\tclusters[" << cars[memberKey].clusterID << "].membersList: " << cl_it->second.membersList.c_str() << endl;
            }
            //creating global-statistics event record
            std::set<MacNodeId> targetSet;
            for(int memberKey : cl_it->second.members)
                targetSet.insert(cars[memberKey].macID);
            stat_->recordNewBroadcast(eventID, targetSet);
            eventID++;
        }
    }
    //V2V_UNICAST_TX_MODE or  V2V_MULTICAST_TX_MODE
    else if(!strcmp(preconfiguredTxMode.c_str(), V2V_UNICAST_TX_MODE) || !strcmp(preconfiguredTxMode.c_str(), V2V_MULTICAST_TX_MODE))
    {
        for(cl_it = clusters.begin(); cl_it != clusters.end(); cl_it++)
        {
            int leaderKey = cl_it->second.members.at(0);

            ClusterizeConfigPacket* pkt = ClusterizePacketBuilder().buildClusterizeConfigPacket(0, 0, eventID, 1, 0, 0, "", "", cars[leaderKey].clusterID, cl_it->second.color.c_str(), cars[leaderKey].txMode.c_str(), cars[leaderKey].following.c_str(), cars[leaderKey].follower.c_str(),cl_it->second.membersList.c_str(), cl_it->second.accelerations);
            //sending to the leader MEClusterizeApp (on the corresponded gate!)

            //sending distanceGaps
            int size = cl_it->second.distancies.size();
            pkt->setDistanciesArraySize(size);
            for(int i=0; i < size; i++)     pkt->setDistancies(i, cl_it->second.distancies.at(i));

            send(pkt, "meAppOut", leaderKey);

            //testing
            std::string txmode = (!strcmp(preconfiguredTxMode.c_str(), V2V_UNICAST_TX_MODE))? V2V_UNICAST_TX_MODE : V2V_MULTICAST_TX_MODE;
            EV << "\nMEClusterizeService::sendConfig - sending ClusterizeConfig to CL: " << cars[leaderKey].symbolicAddress << " (txMode: "<< txmode << ") " << endl;
            EV << "MEClusterizeService::sendConfig - \t\tclusters[" << cars[leaderKey].clusterID << "].membersList: " << cl_it->second.membersList.c_str() << endl << endl;

            //creating global-statistics event record
            std::set<MacNodeId> targetSet;
            for(int memberKey : cl_it->second.members)
                targetSet.insert(cars[memberKey].macID);
            stat_->recordNewBroadcast(eventID,targetSet);
            eventID++;
        }
    }
    //HYBRID_TX_MODE
    else
    {
        //TODO
        //for each cluster in clusters map (cl : clusters)
                //for each cluster member (c : cl.members)
                        //check its txMode (cars[c].txMode) computed by the compute() implementing the algorithm
                        //i.e. if DOWNLINK_UNICAST then send to it
                        //i.e. if V2V_UNICAST or V2V_MULTICAST then to it only if it is the leader!
        //TODO
    }

    // triggering the MEClusterize App to emit statistics (also if the INFO_MEAPP ClsuterizeConfigPacket is sent only to the leader and then propagated!)
    for(cl_it = clusters.begin(); cl_it != clusters.end(); cl_it++)
        for(int memberKey : cl_it->second.members)
        {
            cMessage* trigger = new cMessage("triggerClusterizeConfigStatistics");
            send(trigger, "meAppOut", memberKey);
        }
}

void MEClusterizeService::handleClusterizeInfo(ClusterizeInfoPacket* pkt){

    if(pkt->getTimestamp() >= lastRun)
    {
        //retrieve the cars map key
        int key = pkt->getArrivalGate()->getIndex();
        //updating the cars map entry
        cars[key].id = pkt->getCarOmnetID();
        cars[key].symbolicAddress = pkt->getSourceAddress();
        cars[key].macID = binder_->getMacNodeIdFromOmnetId(cars[key].id);
        cars[key].timestamp = pkt->getTimestamp();
        cars[key].position.x = pkt->getPositionX();
        cars[key].position.y = pkt->getPositionY();
        cars[key].position.z = pkt->getPositionZ();
        cars[key].speed.x = pkt->getSpeedX();
        cars[key].speed.y = pkt->getSpeedY();
        cars[key].speed.z = pkt->getSpeedZ();
        cars[key].acceleration = pkt->getAcceleration();
        cars[key].angularPosition.alpha = pkt->getAngularPositionA();
        cars[key].angularPosition.beta = pkt->getAngularPositionB();
        cars[key].angularPosition.gamma = pkt->getAngularPositionC();
        cars[key].angularSpeed.alpha = pkt->getAngularSpeedA();
        cars[key].angularSpeed.beta = pkt->getAngularSpeedB();
        cars[key].angularSpeed.gamma = pkt->getAngularSpeedC();
        cars[key].isFollower = false;
        cars[key].hasInitialInfo = true;


        //testing
        EV << "MEClusterizeService::handleClusterizeInfo - Updating cars[" << key <<"] --> " << cars[key].symbolicAddress << " (carID: "<< cars[key].id << ") " << endl;
        EV << "MEClusterizeService::handleClusterizeInfo - cars[" << key << "].position = " << "[" << cars[key].position.x << " ; "<< cars[key].position.y << " ; " << cars[key].position.z  << "]" << endl;
        EV << "MEClusterizeService::handleClusterizeInfo - cars[" << key << "].speed = " << "[" << cars[key].speed.x << " ; "<< cars[key].speed.y << " ; " << cars[key].speed.z  << "]" << endl;
        EV << "MEClusterizeService::handleClusterizeInfo - cars[" << key << "].acceleration = " << cars[key].acceleration << endl;
        EV << "MEClusterizeService::handleClusterizeInfo - cars[" << key << "].angularPostion = " << "[" << cars[key].angularPosition.alpha << " ; "<< cars[key].angularPosition.beta << " ; " << cars[key].angularPosition.gamma  << "]" << endl;
        EV << "MEClusterizeService::handleClusterizeInfo - cars[" << key << "].angularSpeed = " << "[" << cars[key].angularSpeed.alpha << " ; "<< cars[key].angularSpeed.beta << " ; " << cars[key].angularSpeed.gamma  << "]" << endl;
    }
    else
    {
        EV << "MEClusterizeService::handleClusterizeInfo - Discarding update from " << pkt->getSourceAddress() << ": too old time-stamp!" << endl;
    }
}

void MEClusterizeService::handleClusterizeStop(ClusterizePacket* pkt){

    //retrieve the cars map key
    int key = pkt->getArrivalGate()->getIndex();
    //erasing the cars map entry
    if(!cars.empty() && cars.find(key) != cars.end())
    {
        std::map<int, car>::iterator it;
        it = cars.find(key);
        if (it != cars.end())
            cars.erase (it);
    }
    //testing
    EV << "MEClusterizeService::handleClusterizeStop - Erasing cars[" << key <<"]" << endl;
}

void MEClusterizeService::handleClusterizeNewCar(ClusterizePacket* pkt){
    int key = pkt->getArrivalGate()->getIndex();
    if(cars.find(key) != cars.end())
    {
        EV << "MEClusterizeService::handleClusterizeNewCar - car["<< cars[key].id << "] already present!" << endl;
        return;
    }
//    //updating the cars map entry
        cars[key].id = pkt->getCarOmnetID();
        cars[key].symbolicAddress = pkt->getSourceAddress();
        cars[key].ipAddress = inet::L3AddressResolver().resolve(cars[key].symbolicAddress.c_str()).str();
        cars[key].macID = binder_->getMacNodeIdFromOmnetId(cars[key].id);
        cars[key].hasInitialInfo = false;

        EV << "MEClusterizeService::handleClusterizeNewCar - car["<< cars[key].id << "] successfully added!" << endl;
        if(cars.size() >= 2 && !selfGet_->isScheduled() && socket.getState() == inet::TCPSocket::CONNECTED)
        {
            EV << "MEClusterizeService::handleClusterizeNewCar - start sending GET" << endl;

            // a cluster could be made, start requesting the user positions if not already scheduled
            scheduleAt(simTime() + 0, selfGet_);
        }
        else if(cars.size() >= 2 && !selfGet_->isScheduled() && !(socket.getState() == inet::TCPSocket::CONNECTED))
        {
            EV << "MEClusterizeService::handleClusterizeNewCar - connecting to the service" << endl;
            connect();
        }
}

void MEClusterizeService::handleTcpMsg()
{
    emit(respTime, (simTime() - t1).dbl());
    updateClusterInfo();
    if(selfGet_->isScheduled())
        cancelEvent(selfGet_);
    double time = exponential(0.05, 2);
    scheduleAt(simTime() + time, selfGet_);

}

//bool MEClusterizeService::parseResponse(std::string& packet, std::map<std::string, std::string>* request)
//{
//    EV_INFO << "MEClusterizeService::parseResponse" << endl;
//    //    std::string packet(packet_);
//        std::vector<std::string> splitting = lte::utils::splitString(packet, "\r\n\r\n"); // bound between header and body
//        std::string header;
//        std::string body;
//
//        if(splitting.size() == 2) //header and body
//        {
//            EV <<"header and body" << endl;
//            header = splitting[0];
//            body   = splitting[1];
//            request->insert( std::pair<std::string, std::string>("body", body) );
//            std::vector<std::string> line;
//            std::vector<std::string> lines = lte::utils::splitString(header, "\r\n");
//            std::vector<std::string>::iterator it = lines.begin();
//
//            line = lte::utils::splitString(*it, " ");  // Response-Line e.g HTTP/1.1 200 OK
//            if(line.size() < 3 ){
//               // Http::send400Response(socket);
//                return false;
//            }
//            if(!Http::ceckHttpVersion(line[0])){
//               // Http::send505Response(socket);
//                return false;
//            }
//
//            if(line.size() == 3)
//            {
//                request->insert( std::pair<std::string, std::string>("http", line[0]) );
//                request->insert( std::pair<std::string, std::string>("code", line[1]) );
//                request->insert( std::pair<std::string, std::string>("reason", line[2]) );
//            }
//
//            else if (line.size() == 4)
//            {
//                request->insert( std::pair<std::string, std::string>("http", line[0]) );
//                request->insert( std::pair<std::string, std::string>("code", line[1]) );
//                request->insert( std::pair<std::string, std::string>("reason", line[2]+line[3]) );
//
//            }
//            for(++it; it != lines.end(); ++it) {
//                line = lte::utils::splitString(*it, ": ");
//                if(line.size() == 2)
//                    request->insert( std::pair<std::string, std::string>(line[0], line[1]) );
//                else
//                {
//                    //Http::send400Response(socket); // bad request
//                    return false;
//                }
//            }
//        }
//        else if(splitting.size() == 1) // only header or only body
//        {
//            body   = splitting[0];
//            request->insert( std::pair<std::string, std::string>("body", body) );
//        }
//
//        return true;
//}


void MEClusterizeService::updateCarInfo(nlohmann::json& userInfo)
{
//    if(simTime() > 32)
//        return;
    EV << "MEClusterizeService::updateCarInfo" << endl;
    std::map<int, car>::iterator it;
    //
    for(it = cars.begin(); it != cars.end(); it++)
    {
       std::vector<std::string> address = lte::utils::splitString(userInfo["address"], ":");
       if(address.size()!= 2) //must be acr:value
       {
           continue;
       }
       std::string ipAddress = address[1];
       if(it->second.ipAddress.compare(ipAddress) == 0)
       {
           EV << "MEClusterizeService::updateCarInfo - Car found" << endl;
           double timestamp = userInfo["timeStamp"];
           if(timestamp >= lastRun.dbl())
           {
                double positionX = userInfo["locationInfo"]["x"];
                double positionY = userInfo["locationInfo"]["y"];
                double positionZ = userInfo["locationInfo"]["z"];

                EV << "New Position: [" << positionX << "; " << positionY << "; " << positionZ << "]" << endl;
                EV << "Old Position" << it->second.position << endl;

                bool speed  = par("speed").boolValue();
                if(speed == true)
                {
                    it->second.speed.x = userInfo["locationInfo"]["speed"]["x"];
                    it->second.speed.y = userInfo["locationInfo"]["speed"]["y"];
                    it->second.speed.z = userInfo["locationInfo"]["speed"]["z"];

                    inet::Coord direction = it->second.speed;
                    direction.normalize();
                    it->second.angularPosition.alpha = -1* atan2(-direction.y, direction.x);
                    it->second.angularPosition.beta = asin(direction.z);
                    it->second.angularPosition.gamma = 0;

                    it->second.hasInitialInfo = true;
                }

                else
                {
                    if(it->second.hasInitialInfo == false){
                        // first time the service has info about UE
                        it->second.hasInitialInfo = true;
                        it->second.oldPosition. x = positionX;
                        it->second.oldPosition. y = positionY;
                        it->second.oldPosition. z = positionZ;


                    }
                    else{
                        double deltaTime = (timestamp-it->second.timestamp.dbl());

                        inet::Coord deltaPosition;
                        deltaPosition.x = (positionX - it->second.oldPosition.x);
                        deltaPosition.y = (positionY - it->second.oldPosition.y);
                        deltaPosition.z = (positionZ - it->second.oldPosition.z);

                        inet::Coord newSpeed;
                        newSpeed.x = deltaPosition.x/deltaTime;
                        newSpeed.y = deltaPosition.y/deltaTime;
                        newSpeed.z = deltaPosition.z/deltaTime;

                        double speedT = userInfo["locationInfo"]["speed"]["y"];

                        double diff = speedT - newSpeed.y;

                        errorSpeed.record(diff);


                        inet::Coord newAcc;
                        newAcc.x = (newSpeed.x - it->second.oldSpeed.x)/deltaTime;
                        newAcc.y = (newSpeed.y - it->second.oldSpeed.y)/deltaTime;
                        newAcc.z = (newSpeed.z - it->second.oldSpeed.z)/deltaTime;

                        //it->second.acceleration = newAcc.length();


                        EV << "delta time: " << deltaTime << endl;
                        EV << "delta Position: "<< deltaPosition << endl;
                        EV << "calculated speed: " << newSpeed << endl;
                        EV << "speed diff: " << diff << endl;


                        it->second.speed = newSpeed;

                        inet::Coord direction = it->second.speed;
                        direction.normalize();
                        it->second.angularPosition.alpha = -1* atan2(-direction.y, direction.x);
                        it->second.angularPosition.beta = asin(direction.z);
                        it->second.angularPosition.gamma = 0;


                        it->second.oldPosition. x = positionX;
                        it->second.oldPosition. y = positionY;
                        it->second.oldPosition. z = positionZ;
                        it->second.oldSpeed = newSpeed;

//                        it->second.speed.x = speedX;
//                        it->second.speed.y = speedY;
//                        it->second.speed.z = speedZ;
                        //testing

                        it->second.hasInitialInfo = true;
                    }
                }

                it->second.position.x = positionX;
                it->second.position.y = positionY;
                it->second.position.z = positionZ;
                it->second.timestamp  = timestamp;
                it->second.isFollower = false;

//              EV << "Position: [" << it->second.position.x << "; " << it->second.position.y << "; " << it->second.position.z << "]" << endl;
//              EV << "Speed: [" << it->second.speed.x << ", " << it->second.speed.y << ", " << ", " << it->second.speed.z << "]" << endl;

                //testing
                EV << "MEClusterizeService::updateCarInfo - Updating cars[" << it->first <<"] --> " << it->second.symbolicAddress << " (carID: "<< it->second.id << ") " << endl;
                EV << "MEClusterizeService::updateCarInfo - cars[" << it->first << "].position = " << "[" << it->second.position.x << " ; "<< it->second.position.y << " ; " << it->second.position.z  << "]" << endl;
                EV << "MEClusterizeService::updateCarInfo - cars[" << it->first << "].speed = " << "[" << it->second.speed.x << " ; "<< it->second.speed.y << " ; " << it->second.speed.z  << "]" << endl;
                EV << "MEClusterizeService::updateCarInfo - cars[" << it->first << "].acceleration = " << it->second.acceleration << endl;
                EV << "MEClusterizeService::updateCarInfo - cars[" << it->first << "].angularPostion = " << "[" << it->second.angularPosition.alpha << " ; "<< it->second.angularPosition.beta << " ; " << it->second.angularPosition.gamma  << "]" << endl;
//              EV << "MEClusterizeService::updateCarInfo - cars[" << it->first << "].angularSpeed = " << "[" << it->second.angularSpeed.alpha << " ; "<< it->second.angularSpeed.beta << " ; " << cars[key].angularSpeed.gamma  << "]" << endl;

           }
           else
           {
               EV << "EClusterizeService::updateCarInfo - Information is too old! - Discarded" << endl;
           }
           break; //next user
       }
    }
    return;
}

void MEClusterizeService::updateClusterInfo(){

    nlohmann::json jsonBody;
    try
    {
        EV << "MEClusterizeService::updateClusterInfo"<< endl;// - message response message "<< receivedMessage.at("body") << endl;
        jsonBody = nlohmann::json::parse(receivedMessage.at("body")); // get the JSON structure
    }
    catch(nlohmann::detail::parse_error e)
    {
        EV <<  e.what() << endl;
        // body is not correctly formatted in JSON, manage it
        return;
    }
    if(jsonBody["userInfo"].is_array())
    {
        nlohmann::json userInfoVector = jsonBody["userInfo"];
        int vectorSize = userInfoVector.size();
        for(int i = 0; i < vectorSize; ++i)
        {
            updateCarInfo(userInfoVector.at(i));
        }
    }
    else
    {
        //only one user
        updateCarInfo(jsonBody["userInfo"]);

    }
}

std::string MEClusterizeService::getCarsAddressesParameters()
{
    std::string parameters;
    std::map<int, car>::iterator it;
    for(it = cars.begin(); it != cars.end(); it++)
    {
        std::string add = "acr:"+ it->second.ipAddress;
        if(it ==  cars.begin())
            parameters += add;
        else
            parameters +=  "," + add;
    }
    EV << "MEClusterizeService::getCarsAddressesParameters - Parameters: " << parameters << endl;
    return parameters;

}

void MEClusterizeService::established(int connId)
{
    EV << "Socket Established" << endl;
    // a cluster could be made, start requesting the user positions if not already scheduled
    if(!selfGet_->isScheduled())
        scheduleAt(simTime() + 0, selfGet_);
}


void MEClusterizeService::finish()
{
    cancelAndDelete(selfGet_);
//    if(socket.getState() == TCPSocket::CONNECTED)
//        socket.close();
}
