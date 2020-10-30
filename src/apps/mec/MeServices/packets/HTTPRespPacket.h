//
//
// @ Alessandro Noferi

//
//

#ifndef __INET_HTTPRESPPACKET_H
#define __INET_HTTPRESPPACKET_H

#include "inet/common/INETDefs.h"
#include "apps/mec/MeServices/packets/HTTPRespPacket_m.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"

//enum response {OK, BAD_REQ, UNAUTH,  FORBIDDEN, NOT_FOUND, NOT_ACC, TOO_REQS, BAD_METHOD, HTTP_NOT_SUPPORTED};
enum response {NULLE};
/**
 * Message that carries raw bytes. Used with emulation-related features.
 */
class HTTPRespPacket : public inet::HTTPRespPacket_Base
{
  private:
    ::omnetpp::opp_string HttpVersion = "HTTP/1.1 ";
    ::omnetpp::opp_string payload;
    ::omnetpp::opp_string method;
    ::omnetpp::opp_string contentType;
    ::omnetpp::opp_string connection;


  public:
    /**
     * Constructor
     */
    HTTPRespPacket(response res_,const char* contType = "application/json") : HTTPRespPacket_Base("response", 0)
    {
        payload = "";
        setResCode(res_);
        setContentType(contType);
        connection = "keep-alive";
    }

    /**
     * Copy constructor
     */
    HTTPRespPacket(const HTTPRespPacket& other) : HTTPRespPacket_Base(other) {}

    /**
     * operator =
     */
    HTTPRespPacket& operator=(const HTTPRespPacket& other) { HTTPRespPacket_Base::operator=(other); return *this; }

    /**
     * Creates and returns an exact copy of this object.
     */
    virtual HTTPRespPacket *dup() const override { return new HTTPRespPacket(*this); }

    /**
     * Set data from buffer
     * @param ptr: pointer to buffer
     * @param length: length of data
     */
    virtual void setDataFromBuffer(const void *ptr, unsigned int length);

    /**
     * Add data from buffer to the end of existing content
     * @param ptr: pointer to input buffer
     * @param length: length of data
     */
    virtual void addDataFromBuffer(const void *ptr, unsigned int length);

    /**
     * Copy data content to buffer
     * @param ptr: pointer to output buffer
     * @param length: length of buffer, maximum of copied bytes
     * @return: length of copied data
     */
    virtual unsigned int copyDataToBuffer(void *ptr, unsigned int length) const;

    /**
     * Truncate data content
     * @param length: The number of bytes from the beginning of the content be removed
     * Generate assert when not have enough bytes for truncation
     */
    virtual void removePrefix(unsigned int length);

    // setters to create the response packet
    void setResCode(response);
    void setContentType(const char *);
    void setConnection(const char *);
    void setHeaderField(const char *); //generic field
    void addNewLine();


    void setBodyOK(const inet::Coord& pos);
    void setBody(const std::string& body);

    void setBodyNOT_FOUND(const std::string& reason);

    void send(inet::TCPSocket *socket);

    ::omnetpp::opp_string& getPacket();
    inet::RawPacket* getRawPacket();
};



#endif // ifndef __INET_HTTPRESPPACKET_H

