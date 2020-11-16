#include "apps/mec/MeServices/packets/HttpResponsePacket.h"
#include "inet/common/INETDefs.h"



HTTPResponsePacket::HTTPResponsePacket(const char *name, short kind): HTTPResponsePacket_Base(name,kind)
{
     setContentType("application/json");
     setConnection("keep-alive");
     setBody("\r\n");
     setPayload("");
}
HTTPResponsePacket::HTTPResponsePacket(const responseCode res, const char *name, short kind): HTTPResponsePacket_Base(name, kind)
{
    setStatus(res);
    setContentType("application/json");
    setConnection("keep-alive");
    setBody("\r\n");
    setPayload("");

}

void HTTPResponsePacket::setStatus(const responseCode res){
    ::omnetpp::opp_string response;
    switch(res) {
        case(OK):
            response = "200 OK";
            break;
        case(BAD_REQ):
            response = "400 BadRequest";
            break;
        case(UNAUTH):
            response = "401 Unauthorized";
            break;
        case(FORBIDDEN):
            response = "403 Forbidden";
            break;
        case(NOT_FOUND):
            response = "404 Not Found";
            break;
        case(NOT_ACC):
            response = "406 Not Acceptable";
            break;
        case(TOO_REQS):
            response = "429 Too Many Requests";
            break;
        case(BAD_METHOD):
                response = "405 Method Not Allowed";
                break;
        case(HTTP_NOT_SUPPORTED):
                response = "505 HTTP Version Not Supported";
                break;
        default:
            throw cRuntimeError("Response code not allowed");
    }
    this->status = httpVersion + response + "\r\n";
}

void HTTPResponsePacket::setContentType(const char* contentType_){
    contentType = ::omnetpp::opp_string("Content-Type: ") + ::omnetpp::opp_string(contentType_) + "\r\n";
}

void HTTPResponsePacket::setConnection(const char* connection_){
    connection = ::omnetpp::opp_string("Connection: ") + ::omnetpp::opp_string(connection_) + "\r\n";
}

void HTTPResponsePacket::setBody(const char * body_){
    body = ::omnetpp::opp_string("\r\n") + ::omnetpp::opp_string(body_) + "\r\n";
}

void HTTPResponsePacket::setBody(const std::string& body_){
    body = ::omnetpp::opp_string("\r\n") + ::omnetpp::opp_string(body_) + "\r\n";
}

void HTTPResponsePacket::setHeaderField(const char* hfield){
    headerFields.push_back(hfield);
}

const char* HTTPResponsePacket::getPayload(){
    this->payload = status + contentType + connection;
    if(!headerFields.empty()){
        std::vector<const char *>::const_iterator it = headerFields.begin();
        std::vector<const char *>::const_iterator end = headerFields.end();
        for(; it != end; ++it){
            payload += ::omnetpp::opp_string(*it) + "\r\n";
        }
    }
        this->payload += body;

    return this->payload.c_str();
}


