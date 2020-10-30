// @author Alessandro Noferi
//

#ifndef __HTTPUTILS_H
#define __HTTPUTILS_H


#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "apps/mec/MeServices/packets/HttpResponsePacket.h"
#include <string>

namespace Http {

void sendPacket(HTTPResponsePacket& pck, inet::TCPSocket *socket);

bool ceckHttpVersion(std::string& httpVersion);

void send200Response(inet::TCPSocket *socket, const char* body);

void send405Response(inet::TCPSocket *socket, const char *methods =  "" );
void send400Response(inet::TCPSocket *socket);
void send505Response(inet::TCPSocket *socket);







}




#endif
