//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include "inet/applications/restServer/packets/RestPacket.h"


namespace inet {

void RestPacket::setDataFromBuffer(const void *ptr, unsigned int length)
{
    restPacket.setDataFromBuffer(ptr, (unsigned int)length);
}

void RestPacket::addDataFromBuffer(const void *ptr, unsigned int length)
{
    restPacket.addDataFromBuffer(ptr, length);
}

unsigned int RestPacket::copyDataToBuffer(void *ptr, unsigned int length) const
{
    return restPacket.copyDataToBuffer(ptr, length);
}

void RestPacket::removePrefix(unsigned int length)
{
    restPacket.removePrefix(length);
}

void RestPacket::createRequest(){
    ::omnetpp::opp_string request = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\n\t\"Hello\" : \"World\"\n}";
    this->restPacket.setDataFromBuffer(request.c_str(), request.size());
    this->restPacket.setByteLength(request.size());
    // TODO add parameters!
}
} // namespace inet

