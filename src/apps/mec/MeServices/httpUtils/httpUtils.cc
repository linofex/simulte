
#include "httpUtils.h"
#include "apps/mec/MeServices/packets/HTTPRespPacket.h"

namespace Http {


    bool ceckHttpVersion(std::string& httpVersion){
        // HTTP/1.1
        // HTTP/2
        // HTTP/3
        return (httpVersion.compare("HTTP/1.1") || httpVersion.compare("HTTP/2"));
    }


    void send200Response(inet::TCPSocket *socket, const char* body){

    }

    void send405Response(inet::TCPSocket *socket, const char* methods){
        HTTPRespPacket resp = HTTPRespPacket(BAD_METHOD);
        resp.setResCode(BAD_METHOD);
        resp.setContentType("application/json");
        if(strcmp (methods,"") == 0)
            resp.setHeaderField("Allow: GET, POST, DELETE, PUT");
        else{
            int size = strlen(methods);
            char header[7+size+1]; // Allow:_+ methods + 0
            strcpy (header,"Allow :");
            strcat (header,methods);
            resp.setHeaderField(header);
        }
        resp.addNewLine();
        resp.send(socket);
    }



    void send400Response(inet::TCPSocket *socket){
        HTTPRespPacket resp = HTTPRespPacket(BAD_REQ);
        resp.setResCode(BAD_REQ);
        resp.setContentType("application/json");
        resp.addNewLine();
        resp.send(socket);
    }
    void send505Response(inet::TCPSocket *socket){
        HTTPRespPacket resp = HTTPRespPacket(BAD_REQ);
        resp.setResCode(BAD_REQ);
        resp.setContentType("application/json");
        resp.addNewLine();
        resp.send(socket);
    }
}
