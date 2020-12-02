#include "MecCommon.h"

Trigger getTrigger(std::string& trigger)
{
    if(trigger.compare("PERIODICAL") == 0)
        return PERIODICAL;
    if(trigger.compare("UTI_80") == 0)
        return UTI_80;
    if(trigger.compare("DL_TPU_1") == 0)
        return DL_TPU_1;

    throw cRuntimeError("getTrigger - trigger %s not exist", trigger.c_str());
}
