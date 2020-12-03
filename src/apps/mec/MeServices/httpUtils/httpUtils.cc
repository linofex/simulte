
#include "httpUtils.h"

#include "inet/common/RawPacket.h"
#include "inet/common/INETDefs.h"
#include "apps/mec/MeServices/httpUtils/json.hpp"

namespace Http {
    /* ProblemDetail structure from RFC 7807
     * {
     * "type": "https://example.com/probs/out-of-credit",
     * "title": "You do not have enough credit.",
     * "detail": "Your current balance is 30, but that costs 50.",
     * "instance": "/account/12345/msgs/abc",
     * "balance": 30,
     * "accounts": ["/account/12345","/account/67890"],
     * "status": 403
     * }
     *
     *
     * {
     *  "type": "https://example.net/validation-error",
     *  "title": "Your request parameters didn't validate.",
     *  "invalid-params": [ {
     *      "name": "age",
     *      "reason": "must be a positive integer"
     *   },
     *   {
     *   "name": "color",
     *   "reason": "must be 'green', 'red' or 'blue'"}
     *   ]
     *}
     *
     *  Content-Type: application/problem+json
     *
     *  RNI API
     *
     *  {
     *      "ProblemDetails": {
     *          "type": "string",
     *          "title": "string",
     *          "status": 0,
     *          "detail": "string",
     *          "instance": "string"
     *      }
     *  }
     *
     *
     */



    void sendPacket(const char* payload, inet::TCPSocket *socket){
        inet::RawPacket *res = new inet::RawPacket("response");
    //        const char* payload = pck.getPayload();
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
        sendPacket(resp.getPayload(), socket);
    }

    void send201Response(inet::TCPSocket *socket, const char* body){
            HTTPResponsePacket resp = HTTPResponsePacket(CREATED);
            resp.setBody(body);
            sendPacket(resp.getPayload(), socket);
    }

    void send201Response(inet::TCPSocket *socket, const char* body, std::pair<std::string, std::string>& header){
            HTTPResponsePacket resp = HTTPResponsePacket(CREATED);
            resp.setBody(body);
            resp.setHeaderField(header.first, header.second);
            sendPacket(resp.getPayload(), socket);
    }

    void send201Response(inet::TCPSocket *socket, const char* body,std::map<std::string, std::string>& headers){
            HTTPResponsePacket resp = HTTPResponsePacket(CREATED);
            resp.setBody(body);
            std::map<std::string, std::string>::iterator it = headers.begin();
            std::map<std::string, std::string>::iterator end = headers.end();
            for(; it != end ; ++it)
            {
                resp.setHeaderField(it->first, it->second);
            }
            sendPacket(resp.getPayload(), socket);
    }


    void send204Response(inet::TCPSocket *socket){
        HTTPResponsePacket resp = HTTPResponsePacket(NO_CONTENT);
        sendPacket(resp.getPayload(), socket);
    }

    void send405Response(inet::TCPSocket *socket, const char* methods){
        HTTPResponsePacket resp = HTTPResponsePacket(BAD_METHOD);
        if(strcmp (methods,"") == 0)
            resp.setHeaderField("Allow: ", "GET, POST, DELETE, PUT");
        else{
            resp.setHeaderField("Allow: ", std::string(methods));
        }
        sendPacket(resp.getPayload(), socket);
    }

    void send400Response(inet::TCPSocket *socket){
        HTTPResponsePacket resp = HTTPResponsePacket(BAD_REQ);
        resp.setBody("{ \"send400Response\" : \"TODO implement ProblemDetails\"}");
        sendPacket(resp.getPayload(), socket);
    }

    void send404Response(inet::TCPSocket *socket){
        HTTPResponsePacket resp = HTTPResponsePacket(NOT_FOUND);
        resp.setBody("{ \"send404Response\" : \"TODO implement ProblemDetails\"}");
        sendPacket(resp.getPayload(), socket);
    }

    void send505Response(inet::TCPSocket *socket){
        HTTPResponsePacket resp = HTTPResponsePacket(HTTP_NOT_SUPPORTED);
        resp.setBody("{TODO implement ProblemDetails}");

        sendPacket(resp.getPayload(), socket);
    }

    void sendPostRequest(inet::TCPSocket *socket, const char* body, const char* host, const char* uri)
    {
        HTTPRequestPacket req = HTTPRequestPacket(POST);
        req.setHost(host);
        req.setUri(uri);
        req.setLength(strlen(body));
        req.setBody(body);
        sendPacket(req.getPayload(), socket);

    }



    }
