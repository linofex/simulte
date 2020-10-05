// author @aleassandro noferi

#ifndef __INET_RESTPACKET_H
#define __INET_RESTPACKET_H

#include "inet/common/INETDefs.h"
#include "inet/applications/restServer/packets/RestPacket_m.h"

namespace inet {

/**
 * Message that carries raw bytes. Used with emulation-related features.
 */
class INET_API RestPacket : public RestPacket_Base
{
  public:

    /**
     * Constructor
     */
    RestPacket(const char *name = nullptr, int kind = 0) : RestPacket_Base(name, kind) {}

    /**
     * Copy constructor
     */
    RestPacket(const RestPacket& other) : RestPacket_Base(other) {}

    /**
     * operator =
     */
    RestPacket& operator=(const RestPacket& other) { RestPacket_Base::operator=(other); return *this; }

    /**
     * Creates and returns an exact copy of this object.
     */
    virtual RestPacket *dup() const override { return new RestPacket(*this); }

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


    virtual void createRequest();
};

} // namespace inet

#endif // ifndef __INET_RESTPACKET_H

