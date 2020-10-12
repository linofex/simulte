//
//
// @ Alessandro Noferi

//
//

#ifndef __INET_HTTPRESPPACKET_H
#define __INET_HTTPRESPPACKET_H

#include "inet/common/INETDefs.h"
#include "apps/mec/restServer/packets/HTTPRespPacket_m.h"
#include "inet/common/geometry/common/Coord.h"


enum response {OK, BAD_REQ, UNAUTH,  FORBIDDEN, NOT_FOUND, NOT_ACC, TOO_REQS};

/**
 * Message that carries raw bytes. Used with emulation-related features.
 */
class HTTPRespPacket : public inet::HTTPRespPacket_Base
{
  private:
    ::omnetpp::opp_string HttpVersion = "HTTP/1.1 ";
    ::omnetpp::opp_string payload;
  public:
    /**
     * Constructor
     */
    HTTPRespPacket(const char *name = nullptr, int kind = 0) : HTTPRespPacket_Base(name, kind) {}

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
    void setBody(const inet::Coord& pos);
    ::omnetpp::opp_string& getPacket();
    inet::RawPacket* getRawPacket();
};



#endif // ifndef __INET_HTTPRESPPACKET_H

