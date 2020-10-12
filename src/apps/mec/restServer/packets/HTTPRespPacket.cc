//
// @ Alessandro Noferi
//

#include "apps/mec/restServer/packets/HTTPRespPacket.h"


void HTTPRespPacket::setDataFromBuffer(const void *ptr, unsigned int length)
{
    byteArray.setDataFromBuffer(ptr, (unsigned int)length);
}

void HTTPRespPacket::addDataFromBuffer(const void *ptr, unsigned int length)
{
    byteArray.addDataFromBuffer(ptr, length);
}

unsigned int HTTPRespPacket::copyDataToBuffer(void *ptr, unsigned int length) const
{
    return byteArray.copyDataToBuffer(ptr, length);
}

void HTTPRespPacket::removePrefix(unsigned int length)
{
    byteArray.removePrefix(length);
}

void HTTPRespPacket::setResCode(const response res){
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
        default:
            throw cRuntimeError("Response code not allowed");
    }
    ::omnetpp::opp_string resCode = HttpVersion + response + "\r\n";
    payload =  resCode;
    byteArray.setDataFromBuffer(resCode.c_str(), resCode.size());
    addByteLength(resCode.size());
}

void HTTPRespPacket::setContentType(const char * conType){
    ::omnetpp::opp_string field = ::omnetpp::opp_string("Content-Type: ") + ::omnetpp::opp_string(conType) + "\r\n";
    payload += field;
    byteArray.addDataFromBuffer(field.c_str(), field.size());
    addByteLength(field.size());
}

void HTTPRespPacket::setConnection(const char *conn){
    ::omnetpp::opp_string field = ::omnetpp::opp_string("Connection: ") + ::omnetpp::opp_string(conn) + "\r\n";
    payload += field;
    byteArray.addDataFromBuffer(field.c_str(), field.size());
    addByteLength(field.size());

}

void HTTPRespPacket::setHeaderField(const char *field_){
    ::omnetpp::opp_string field = ::omnetpp::opp_string(field_) + "\r\n";
    payload += field;
    byteArray.setDataFromBuffer(field.c_str(), field.size());
    setByteLength(field.size());

}
void HTTPRespPacket::addNewLine(){
    byteArray.addDataFromBuffer("\r\n", 2);
    addByteLength(2);
}

void HTTPRespPacket::setBodyOK(const inet::Coord& pos){
    std::ostringstream strs;
    std::string body;
    strs << "\r\n" << "{\n  \"userInfo\": {\n    \"address\": \"INDIRIZZO\",\n    \"locationInfo\": {\n      \"x\": " << pos.x << ",\n      \"y\": " << pos.y << ",\n      \"z\": " << pos.z << "\n    }\n  }\n}";
    body = strs.str();
    byteArray.addDataFromBuffer(body.c_str(), body.size());
    addByteLength(body.size());
    payload += body;
}

void HTTPRespPacket::setBodyNOT_FOUND(const std::string& reason){
    std::ostringstream strs;
    std::string body;
    strs << "\r\n" << "{\n  \"ProblemDetails\": {\n    \"reason\": \""<< reason << "\"\n  }\n}";
    body = strs.str();
    byteArray.addDataFromBuffer(body.c_str(), body.size());
    addByteLength(body.size());
    payload += body;
}


inet::RawPacket* HTTPRespPacket::getRawPacket(){
    return &byteArray;
}

::omnetpp::opp_string& HTTPRespPacket::getPacket(){
    return payload;
}


