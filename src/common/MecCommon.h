#ifndef  _MECCOMMON_H_
#define _MECCOMMON_H_

#include <string>
#include "common/LteCommon.h"


enum RlcBurstStatus
{
    START, STOP, ACTIVE, INACTIVE
};

enum Trigger
{
    L2_MEAS_PERIODICAL, L2_MEAS_UTI_80, L2_MEAS_DL_TPU_1
};


Trigger getTrigger(std::string& trigger);

typedef struct
{
    unsigned int pktCount;
    simtime_t time;
} Delay;

typedef struct
{
    unsigned int pktSizeCount;
    simtime_t time;
} Throughput;

typedef struct {
    unsigned int discarded;
    unsigned int total;
} DiscardedPkts;


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
