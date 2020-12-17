/*
 * UeAppPacket.h
 *
 *  Created on: Dec 14, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_PERFORMANCEEVALUATION_MEAPP_PACKETS_UEAPPPACKET_H_
#define APPS_MEC_PERFORMANCEEVALUATION_MEAPP_PACKETS_UEAPPPACKET_H_

#include "apps/mec/PerformanceEvaluation/MeApp/packets/UeAppPacket_m.h"
#include "common/LteCommon.h"

class UeAppPacket : public UeAppPacket_Base
{
  private:
    inet::Coord coordsAtRequest_;
    inet::Coord coordsAtResponse_;
    double directionAtRequest_;
    double directionAtResponse_;


    void copy(const UeAppPacket& other) { coordsAtRequest_ = other.coordsAtRequest_; coordsAtResponse_ = other.coordsAtResponse_;
    directionAtRequest_ = other.directionAtRequest_; directionAtResponse_= other.directionAtResponse_; }

  public:
    UeAppPacket(const char *name=nullptr, short kind=0) : UeAppPacket_Base(name,kind), coordsAtRequest_(), coordsAtResponse_() {directionAtRequest_ = 0.; directionAtResponse_= 0.;}
    UeAppPacket(const UeAppPacket& other) : UeAppPacket_Base(other) {copy(other);}
    UeAppPacket& operator=(const UeAppPacket& other) {if (this==&other) return *this; UeAppPacket_Base::operator=(other); copy(other); return *this;}
    virtual UeAppPacket *dup() const override {return new UeAppPacket(*this);}

    virtual void setRequestCoord(const inet::Coord& coordsAtRequest) {coordsAtRequest_ = coordsAtRequest;}
    virtual void setResponseCoord(const inet::Coord& coordsAtResponse) {coordsAtResponse_ = coordsAtResponse;}
    virtual void setRequestOrientation(const double orientation){directionAtRequest_ = orientation;}
    virtual void setResponseOrientation(const double orientation){directionAtResponse_ = orientation;}


    virtual inet::Coord getRequestCoord() {return coordsAtRequest_;}
    virtual inet::Coord getResponseCoord() {return coordsAtResponse_;}
    virtual double getRequestOrientation() {return directionAtRequest_;}
    virtual double getResponseOrientation() {return directionAtResponse_;}


    // ADD CODE HERE to redefine and implement pure virtual functions from UeAppPacket_Base
};


#endif /* APPS_MEC_PERFORMANCEEVALUATION_MEAPP_PACKETS_UEAPPPACKET_H_ */
