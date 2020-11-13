
#include "httpUtils.h"
#include "apps/mec/MeServices/packets/HttpResponsePacket.h"
#include "inet/common/RawPacket.h"
#include "inet/common/INETDefs.h"

namespace Http {

    void sendPacket(HTTPResponsePacket& pck, inet::TCPSocket *socket){
        inet::RawPacket *res = new inet::RawPacket("response");
        const char* payload = pck.getPayload();
        res->setDataFromBuffer(payload , strlen(payload));
        res->setByteLength(strlen(payload));
        socket->send(res);
    }


    bool ceckHttpVersion(std::string& httpVersion){
        // HTTP/1.1
        // HTTP/2
        // HTTP/3
        return (httpVersion.compare("HTTP/1.1") || httpVersion.compare("HTTP/2"));
    }


    void send200Response(inet::TCPSocket *socket, const char* body){
        HTTPResponsePacket resp = HTTPResponsePacket(OK);
        resp.setBody(body);
        sendPacket(resp, socket);
    }

    void send405Response(inet::TCPSocket *socket, const char* methods){
        HTTPResponsePacket resp = HTTPResponsePacket(BAD_METHOD);
        if(strcmp (methods,"") == 0)
            resp.setHeaderField("Allow: GET, POST, DELETE, PUT");
        else{
            int size = strlen(methods);
            char header[7+size+1]; // Allow:_+ methods + 0
            strcpy (header,"Allow: ");
            strcat (header,methods);
            resp.setHeaderField(header);
        }
        sendPacket(resp, socket);
    }

    void send400Response(inet::TCPSocket *socket){
        HTTPResponsePacket resp = HTTPResponsePacket(BAD_REQ);
        sendPacket(resp, socket);
    }

    void send404Response(inet::TCPSocket *socket){
        HTTPResponsePacket resp = HTTPResponsePacket(NOT_FOUND);
        sendPacket(resp, socket);
    }

    void send505Response(inet::TCPSocket *socket){
        HTTPResponsePacket resp = HTTPResponsePacket(HTTP_NOT_SUPPORTED);
        sendPacket(resp, socket);
    }
}
