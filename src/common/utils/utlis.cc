

#include "common/utils/utils.h"
namespace utils {

    std::vector<std::string> splitString(std::string str, std::string delim){
        size_t last = 0;
        size_t next = 0;
        std::vector<std::string> splitted;
        std::string line;
        if(str.size() == 0) return splitted;
        while ((next = str.find(delim, last)) != std::string::npos) {
                line = str.substr(last, next-last);
                if(str.size() != 0) splitted.push_back(line);
                last = next + delim.size();
        }
        // if the delim is at the end of the str, an empty str will be added
        // side effect useful for the body of the request
        splitted.push_back(str.substr(last, next-last)); // last token
        return splitted;
    }

    char* getPacketPayload(::omnetpp::cMessage *msg){
        inet::RawPacket *request = check_and_cast<inet::RawPacket *>(msg);
        if (request == 0) throw cRuntimeError("UEWarningAlertApp_rest::handleMessage - \tFATAL! Error when casting to MEAppPacket");
        return request->getByteArray().getDataPtr();
    }

    inet::RawPacket* createUDPPacket(const std::string& payload){
       inet::RawPacket *pck  = new inet::RawPacket("udpPacket");
       pck->setDataFromBuffer(payload.c_str(), payload.size());
       pck->setByteLength(payload.size());
       return pck;
    }


}

