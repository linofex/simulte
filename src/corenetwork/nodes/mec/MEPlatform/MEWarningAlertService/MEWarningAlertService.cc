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

#include "MEWarningAlertService.h"

Define_Module(MEWarningAlertService);

void MEWarningAlertService::initialize(int stage)
{
    EV << "MEWarningAlertService::initialize - stage " << stage << endl;
    cSimpleModule::initialize(stage);
    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    // setting the MEWarningAlertService gate sizes
    int maxMEApps = 0;
    cModule* mePlatform = getParentModule();
    if(mePlatform != NULL){
        cModule* meHost = mePlatform->getParentModule();
        if(meHost->hasPar("maxMEApps"))
                maxMEApps = meHost->par("maxMEApps").longValue();
        else
            throw cRuntimeError("MEWarningAlertService::initialize - \tFATAL! Error when getting meHost.maxMEApps parameter!");

        this->setGateSize("meAppOut", maxMEApps);
        this->setGateSize("meAppIn", maxMEApps);
    }
    else{
        EV << "MEWarningAlertService::initialize - ERROR getting mePlatform cModule!" << endl;
        throw cRuntimeError("MEWarningAlertService::initialize - \tFATAL! Error when getting getParentModule()");
    }

    //retrieving parameters
    dangerEdgeA = inet::Coord(par("dangerEdgeAx").doubleValue(), par("dangerEdgeAy").doubleValue(), par("dangerEdgeAz").doubleValue());
    dangerEdgeB = inet::Coord(par("dangerEdgeBx").doubleValue(), par("dangerEdgeBy").doubleValue(), par("dangerEdgeBz").doubleValue());
    dangerEdgeC = inet::Coord(par("dangerEdgeCx").doubleValue(), par("dangerEdgeCy").doubleValue(), par("dangerEdgeCz").doubleValue());
    dangerEdgeD = inet::Coord(par("dangerEdgeDx").doubleValue(), par("dangerEdgeDy").doubleValue(), par("dangerEdgeDz").doubleValue());

    //drawing the Danger Area
    cPolygonFigure *polygon = new cPolygonFigure("polygon");
    std::vector<cFigure::Point> points;
    points.push_back(cFigure::Point(dangerEdgeA.x, dangerEdgeA.y));
    points.push_back(cFigure::Point(dangerEdgeB.x, dangerEdgeB.y));
    points.push_back(cFigure::Point(dangerEdgeC.x, dangerEdgeC.y));
    points.push_back(cFigure::Point(dangerEdgeD.x, dangerEdgeD.y));
    polygon->setPoints(points);
    polygon->setLineColor(cFigure::RED);
    polygon->setLineWidth(2);
    getSimulation()->getSystemModule()->getCanvas()->addFigure(polygon);


    cOvalFigure *circle = new cOvalFigure("circle");
    circle->setBounds(cFigure::Rectangle(100,120,120,120));
    circle->setLineWidth(2);
    circle->setLineStyle(cFigure::LINE_DOTTED);
    getSimulation()->getSystemModule()->getCanvas()->addFigure(circle);


}

void MEWarningAlertService::handleMessage(cMessage *msg)
{
    EV << "MEWarningAlertService::handleMessage - \n";

    WarningAlertPacket* pkt = check_and_cast<WarningAlertPacket*>(msg);
    if (pkt == 0)
        throw cRuntimeError("MEWarningAlertService::handleMessage - \tFATAL! Error when casting to WarningAlertPacket");

    if(!strcmp(pkt->getType(), INFO_UEAPP))         handleInfoUEWarningAlertApp(pkt);

    delete pkt;
}

void MEWarningAlertService::handleInfoUEWarningAlertApp(WarningAlertPacket* pkt){

    EV << "MEWarningAlertService::handleInfoUEWarningAlertApp - Received " << pkt->getType() << " type WarningAlertPacket from " << pkt->getSourceAddress() << endl;

    inet::Coord uePosition(pkt->getPositionX(), pkt->getPositionY(), pkt->getPositionZ());

    WarningAlertPacket* packet = new WarningAlertPacket();
    packet->setType(INFO_MEAPP);

    if(isInQuadrilateral(uePosition, dangerEdgeA, dangerEdgeB, dangerEdgeC, dangerEdgeD)){

        packet->setDanger(true);

        send(packet, "meAppOut", pkt->getArrivalGate()->getIndex());

        EV << "MEWarningAlertService::handleInfoUEWarningAlertApp - "<< pkt->getSourceAddress() << " is in Danger Area! Sending the " << INFO_MEAPP << " type WarningAlertPacket with danger == TRUE!" << endl;
    }
    else{

        packet->setDanger(false);

        send(packet, "meAppOut", pkt->getArrivalGate()->getIndex());

        EV << "MEWarningAlertService::handleInfoUEWarningAlertApp - "<< pkt->getSourceAddress() << " is not in Danger Area! Sending the " << INFO_MEAPP << " type WarningAlertPacket with danger == FALSE!" << endl;
    }
}

bool MEWarningAlertService::isInTriangle(inet::Coord P, inet::Coord A, inet::Coord B, inet::Coord C){

      //considering all points relative to A
      inet::Coord v0 = B-A;   // B w.r.t A
      inet::Coord v1 = C-A;   // C w.r.t A
      inet::Coord v2 = P-A;   // P w.r.t A

      // writing v2 = u*v0 + v*v1 (linear combination in the Plane defined by vectors v0 and v1)
      // and finding u and v (scalar)

      double den = ((v0*v0) * (v1*v1)) - ((v0*v1) * (v1*v0));

      double u = ( ((v1*v1) * (v2*v0)) - ((v1*v0) * (v2*v1)) ) / den;
      double v = ( ((v0*v0) * (v2*v1)) - ((v0*v1) * (v2*v0)) ) / den;

      // checking if coefficients u and v are constrained in [0-1], that means inside the triangle ABC
      if(u>=0 && v>=0 && u+v<=1)
      {
          //EV << "inside!";
          return true;
      }
      else{
          //EV << "outside!";
          return false;
      }
}

bool MEWarningAlertService::isInQuadrilateral(inet::Coord P, inet::Coord A, inet::Coord B, inet::Coord C, inet::Coord D)
{
      return isInTriangle(P, A, B, C) || isInTriangle(P, A, C, D);
}
