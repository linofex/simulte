//
//
// @ Alessandro Noferi

//
//

#ifndef __INET_HTTPRESPPACKET_H
#define __INET_HTTPRESPPACKET_H

#include "inet/common/INETDefs.h"
#include "inet/applications/restServer/packets/HTTPRespPacket_m.h"

namespace inet {

enum response {OK, BAD_REQ, UNAUTH,  FORBIDDEN, NOT_FOUND, NOT_ACC, TOO_REQS};

/**
 * Message that carries raw bytes. Used with emulation-related features.
 */
class INET_API HTTPRespPacket : public HTTPRespPacket_Base
{
  private:
    ::omnetpp::opp_string HttpVersion = "HTTP/1.1 ";
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
    void setBody();
    RawPacket* getRawPacket();
};

} // namespace inet

#endif // ifndef __INET_HTTPRESPPACKET_H

