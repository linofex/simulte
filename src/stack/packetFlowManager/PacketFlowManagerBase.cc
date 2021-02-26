//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "PacketFlowManagerBase.h"
#include "stack/mac/layer/LteMacBase.h"
#include "stack/pdcp_rrc/layer/LtePdcpRrc.h"
#include "stack/rlc/packet/LteRlcPdu.h"
#include "stack/rlc/packet/LteRlcSdu.h"
#include "stack/mac/packet/LteMacPdu.h"
#include "stack/rlc/LteRlcDefs.h"
#include "stack/rlc/packet/LteRlcDataPdu.h"

#include "common/LteControlInfo.h"

#include <sstream>


//Define_Module(PacketFlowManagerBase);

PacketFlowManagerBase::PacketFlowManagerBase()
{
}

PacketFlowManagerBase::~PacketFlowManagerBase()
{
}

void PacketFlowManagerBase::initialize(int stage)
{
    if (stage == 1)
    {
        LteMacBase *mac= check_and_cast<LteMacBase *>(getParentModule()->getSubmodule("mac"));
        nodeType_ = mac->getNodeType();
        harqProcesses_ = (nodeType_ == UE) ? UE_TX_HARQ_PROCESSES : ENB_TX_HARQ_PROCESSES;
    }
}


void PacketFlowManagerBase::initPdcpStatus(StatusDescriptor* desc, unsigned int pdcp, unsigned int headerSize, simtime_t& arrivalTime){}
void PacketFlowManagerBase::removePdcpBurst(StatusDescriptor* desc, PdcpStatus& pdcpStatus,  unsigned int pdcpSno, bool ack){}

void PacketFlowManagerBase::resetDiscardCounter()
{
    pktDiscardCounterTotal_ = {0,0};
}

void PacketFlowManagerBase::finish()
{
}
