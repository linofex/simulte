#ifndef _CELLINFO_H_
#define _CELLINFO_H_

#include <vector>
#include <omnetpp.h>
#include <map>
#include "../../RNIService/resources/AttributeBase.h"
#include "../../RNIService/resources/Ecgi.h"

class EnodeBStatsCollector;

class CellInfo : public AttributeBase {
  protected:
    EnodeBStatsCollector* collector_; // it has the cellCollector and the map <Ipue -> uecollector>
    Ecgi ecgi_;

  /**
   * 
   * or std::map<ipv4, cellUeInfo>
   * I prefer the pointer to the list of users in the cell to manage better
//   * new/deleted users without the need of take care of them here
//   */
//    UeList* ueList_;
//    //Ecgi ecgi_;

    nlohmann::ordered_json toJsonCell() const; //should be private?
  /* data */


public:
  CellInfo();
  CellInfo(::omnetpp::cModule* eNodeB);
  virtual ~CellInfo();

  nlohmann::ordered_json toJson() const override;
//  nlohmann::ordered_json toJson(std::vector<Ipv4>& uesID) const;

};

#endif // _CELLINFO_H_

