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

MEClusterizeService::~MEClusterizeService(){

}

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

    selfGet_ = new cMessage("GetRequest");
    getPeriod_ = par("getPeriod");

    cMessage *msg = new cMessage("connect");
    scheduleAt(simTime() +0 , msg);

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
    std::string host = socket.getRemoteAddress().str()+":"+std::to_string(socket.getRemotePort());
    Http::sendGetRequest(&socket, body.c_str(), host.c_str(), uri.c_str());
}

/*
 * #################################################################################################################################
 */
void MEClusterizeService::handleMessage(cMessage *msg)
{
    EV << "MEClusterizeService::handleMessage" << endl;
    if (msg->isSelfMessage()){
        if(msg->isName("connect"))
        {
            connect();
            delete msg;
            return;

        }
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
           msg->getKind() == inet::TCP_I_PEER_CLOSED)
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
        cars[key].angularPosition.alpha = 1.5708;

        EV << "MEClusterizeService::handleClusterizeNewCar - car["<< cars[key].id << "] successfully added!" << endl;
        if(cars.size() >= 2 && !selfGet_->isScheduled() && socket.getState() == inet::TCPSocket::CONNECTED)
        {
            // a cluster could be made, start requesting the user positions if not already scheduled
            scheduleAt(simTime() + 0, selfGet_);
        }
}

void MEClusterizeService::socketDataArrived(int, void *, cPacket *msg, bool urgent)
{
        EV << "MEClusterizeService::socketDataArrived" << endl;
        std::string packet = lte::utils::getPacketPayload(msg);
        std::map<std::string, std::string>* res = new std::map<std::string, std::string>;

        bool resp = parseResponse(packet, res);
        EV << "resp: " << resp << endl;
        if(res->find("code") != res->end()) //header plus body(maybe)
        {
            // response with
            if(res->at("code").compare("200") == 0)
            {
                if(res->find("Content-Length") != res->end())
                {
                    responseMessageLength = std::stoi(res->at("Content-Length"));
                    EV << responseMessageLength << endl;
                }
                if(responseMessageLength > 0)
                {
                    responseMessage += res->at("body");
                    responseMessageLength -= res->at("body").length();
                    receivingMessage = true;
                }
            }
            else{
                EV << "NO" << endl;
            }
        }
        else // is only body
        {
            responseMessage += res->at("body");
            responseMessageLength -= res->at("body").length();
        }

        if(responseMessageLength == 0 && receivingMessage == true)
        {
            EV << "MEClusterizeService::socketDataArrived - Completed packet arrived." << endl;
            updateClusterInfo();
            responseMessage.clear();
            receivingMessage = false;
//            //close socket
//            socket.close();
            //reschedule the request
            if(selfGet_->isScheduled())
                cancelEvent(selfGet_);
            scheduleAt(simTime() + getPeriod_, selfGet_);
        }
        else if(responseMessageLength < 0)
            throw cRuntimeError("MEClusterizeService::socketDataArrived - read payload more than Content=Length header");
        delete res;
}

void MEClusterizeService::updateCarInfo(nlohmann::json& userInfo)
{
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
           EV << "MEClusterizeService::updateCarInfo - Car founded" << endl;
           double timestamp = userInfo["timeStamp"];
           if(timestamp >= lastRun.dbl())
           {
                double positionX = userInfo["locationInfo"]["x"];
                double positionY = userInfo["locationInfo"]["y"];
                double positionZ = userInfo["locationInfo"]["z"];

                bool speed  = par("speed").boolValue();
                if(speed == true)
                {
                    it->second.speed.x = userInfo["locationInfo"]["speed"]["x"];
                    it->second.speed.y = userInfo["locationInfo"]["speed"]["y"];
                    it->second.speed.z = userInfo["locationInfo"]["speed"]["z"];
                    it->second.hasInitialInfo = true;
                }

                else
                {
                    if(it->second.hasInitialInfo == false){
                        // first time the service has info about UE
                        it->second.hasInitialInfo = true;

                    }
                    else{

                        double deltaTime = (timestamp-it->second.timestamp.dbl());
                        double speedX = (positionX - it->second.position.x)/deltaTime;
                        double speedY = (positionY - it->second.position.y)/deltaTime;
                        double speedZ = (positionZ - it->second.position.z)/deltaTime;

                        it->second.speed.x = speedX;
                        it->second.speed.y = speedY;
                        it->second.speed.z = speedZ;
                        //testing
                        EV << "delta time: " << deltaTime << endl;
                        it->second.hasInitialInfo = true;
                    }
                }

                it->second.position.x = positionX;
                it->second.position.y = positionY;
                it->second.position.z = positionZ;
                it->second.timestamp  = timestamp;
                it->second.angularPosition.alpha = 1.5708;
                it->second.isFollower = false;

                EV << "Position: [" << it->second.position.x << "; " << it->second.position.y << "; " << it->second.position.z << "]" << endl;
                EV << "Speed: [" << it->second.speed.x << ", " << it->second.speed.y << ", " << ", " << it->second.speed.z << "]" << endl;


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
        EV << "MEClusterizeService::updateClusterInfo - message response message "<< responseMessage << endl;
        jsonBody = nlohmann::json::parse(responseMessage); // get the JSON structure
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

bool MEClusterizeService::parseResponse(std::string& packet, std::map<std::string, std::string>* request)
{
    EV_INFO << "MEClusterizeService::parseResponse" << endl;
    //    std::string packet(packet_);
        std::vector<std::string> splitting = lte::utils::splitString(packet, "\r\n\r\n"); // bound between header and body
        std::string header;
        std::string body;

        if(splitting.size() == 2) //header and body
        {
            EV <<"header and body" << endl;
            header = splitting[0];
            body   = splitting[1];
            request->insert( std::pair<std::string, std::string>("body", body) );
            std::vector<std::string> line;
            std::vector<std::string> lines = lte::utils::splitString(header, "\r\n");
            std::vector<std::string>::iterator it = lines.begin();

            line = lte::utils::splitString(*it, " ");  // Response-Line e.g HTTP/1.1 200 OK
            if(line.size() < 3 ){
               // Http::send400Response(socket);
                return false;
            }
            if(!Http::ceckHttpVersion(line[0])){
               // Http::send505Response(socket);
                return false;
            }

            if(line.size() == 3)
            {
                request->insert( std::pair<std::string, std::string>("http", line[0]) );
                request->insert( std::pair<std::string, std::string>("code", line[1]) );
                request->insert( std::pair<std::string, std::string>("reason", line[2]) );
            }

            else if (line.size() == 4)
            {
                request->insert( std::pair<std::string, std::string>("http", line[0]) );
                request->insert( std::pair<std::string, std::string>("code", line[1]) );
                request->insert( std::pair<std::string, std::string>("reason", line[2]+line[3]) );

            }
            for(++it; it != lines.end(); ++it) {
                line = lte::utils::splitString(*it, ": ");
                if(line.size() == 2)
                    request->insert( std::pair<std::string, std::string>(line[0], line[1]) );
                else
                {
                    //Http::send400Response(socket); // bad request
                    return false;
                }
            }
        }
        else if(splitting.size() == 1) // only header or only body
        {
            body   = splitting[0];
            request->insert( std::pair<std::string, std::string>("body", body) );
        }

        return true;
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

void MEClusterizeService::socketEstablished(int, void *)
{
    EV << "Socket Established" << endl;
}

void MEClusterizeService::socketPeerClosed(int, void *)
{
    std::cout<<"Closed connection from: "  << endl;//<< sock->getRemoteAddress()<< std::endl;
    socket.close();
}

void MEClusterizeService::socketClosed(int, void *)
{
    std::cout <<"Removed socket of: " << endl;// << sock->getRemoteAddress() << " from map" << std::endl;
}

void MEClusterizeService::socketFailure(int, void *, int code)
{
    std::cout <<"Socket of: "  << endl;//<< sock->getRemoteAddress() << " failed. Code: " << code << std::endl;
}


