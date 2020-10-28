#include <omnetpp.h>
#include "apps/mec/MeServices/packets/HttpResponsePacket_m.h"

class HTTPResponsePacket : public HTTPResponsePacket_Base
{
    public:
        virtual const char * getStatus() const override;
        virtual void setStatus(const char * status) override;
        virtual void setStatus(const response res) override;
        virtual const char * getConnection() const override;
        virtual void setConnection(const char * connection) override;
        virtual const char * getContentType() const;
        virtual void setContentType(const char * contentType);
        virtual const char * getBody() const;
        virtual void setBody(const char * body);
        virtual const char * getPayload() const;
        virtual void setPayload(const char * payload);

};
