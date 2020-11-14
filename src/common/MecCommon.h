#ifndef  _MECCOMMON_H_
#define _MECCOMMON_H_

#include <string>
#include "common/LteCommon.h"

namespace mec {

    struct AssociateId 
    {
        std::string type;
        std::string value;
    };


    // write defs
    struct Plmn
    {
        std::string mcc;
        std::string mnc;
    };

    struct Ecgi
    {
        Plmn plmn;
        std::string cellId;
    };

    struct Timestamp
    {
        int secods;
        int nanoSecods;
        
    };
    
}

#endif
