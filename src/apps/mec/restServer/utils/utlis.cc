

#include "inet/applications/restServer/utils/utils.h"

namespace inet {

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
            splitted.push_back(str.substr(last, next-last)); // last token
            return splitted;
        }


    }
}
