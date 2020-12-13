// @author Alessandro Noferi
//

#ifndef __HTTPUTILS_H
#define __HTTPUTILS_H


#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "apps/mec/MeServices/packets/response/HttpResponsePacket.h"
#include "apps/mec/MeServices/packets/request/HttpRequestPacket.h"

#include <string>

namespace Http {

enum DataType {REQUEST, RESPONSE, UNKNOWN};

void sendPacket(const char* pck, inet::TCPSocket *socket);

bool ceckHttpVersion(std::string& httpVersion);

void send200Response(inet::TCPSocket *socket, const char *body);
void send201Response(inet::TCPSocket *socket, const char *body);
void send201Response(inet::TCPSocket *socket, const char* body, std::pair<std::string, std::string>& header);
void send201Response(inet::TCPSocket *socket, const char* body, std::map<std::string, std::string>& headers);
void send204Response(inet::TCPSocket *socket);
void send405Response(inet::TCPSocket *socket, const char *methods =  "" );
void send400Response(inet::TCPSocket *socket);
void send404Response(inet::TCPSocket *socket);

void send505Response(inet::TCPSocket *socket);


void sendPostRequest(inet::TCPSocket *socket, const char* body, const char* host, const char* uri);
void sendGetRequest(inet::TCPSocket *socket, const char* body, const char* host, const char* uri);






}




#endif
