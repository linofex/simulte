//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "corenetwork/statsCollector/StatsCollector.h"


using namespace std;

Define_Module(StatsCollector);

StatsCollector::StatsCollector(){}

void StatsCollector::initialize(int stage)
{
    if (stage == inet::INITSTAGE_LOCAL)
       {
        ueCollectors_.clear();
        eNBCollector_.initializeParameters(this);
       }
}

ENodeBCollector* StatsCollector::getENodeBCollector()
{
    return &eNBCollector_;
}
UeCollectors* StatsCollector::getUeCollectors()
{
    return &ueCollectors_;
}
UeCollector* StatsCollector::getUeCollector(MacNodeId id)
{
    UeCollectors::iterator it = ueCollectors_.find(id);
    if(it == ueCollectors_.end()) return nullptr;
    return it->second;
}
void StatsCollector::addUe(MacNodeId id)
{
    if(ueCollectors_.find(id) != ueCollectors_.end())
        ueCollectors_.insert(std::pair<MacNodeId,UeCollector*>(id, new UeCollector(this)));
    else
        throw cRuntimeError ("StatsCollector::attachUe - ue [%d] already has a collector", id);
}
void StatsCollector::attachUe(MacNodeId id, UeCollector* collector)
{
    if(ueCollectors_.find(id) != ueCollectors_.end())
        ueCollectors_.insert(std::pair<MacNodeId,UeCollector*>(id, collector));
    else
        throw cRuntimeError ("StatsCollector::attachUe - ue [%d] already has a collector", id);
}

UeCollector* StatsCollector::detachUe(MacNodeId id)
{
    UeCollectors::iterator it = ueCollectors_.find(id);
    if(it != ueCollectors_.end())
    {
        ueCollectors_.erase(it); //or point to NULL, in case it comes back?
        return it->second;
    }
    return nullptr;
}

void StatsCollector::deleteUe(MacNodeId id){
    UeCollectors::iterator it = ueCollectors_.find(id);
    if(it != ueCollectors_.end()){
        delete it->second;
        ueCollectors_.erase(it);
    }
}

StatsCollector::~StatsCollector(){
    UeCollectors::iterator it = ueCollectors_.begin();
    UeCollectors::iterator end = ueCollectors_.end();
    for(; it != end ; ++it){
        deleteUe(it->first);
    }
}


