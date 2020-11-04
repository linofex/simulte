#ifndef _CELLINFO_H_
#define _CELLINFO_H_

#include "AttributeBase.h"

// #include "Ecgi.h"
//#include "CellUeInfo.h"
#include <vector>
#include <omnetpp.h>
#include <map>

#include "corenetwork/statsCollector/ENodeBCollector.h"


class CellInfo : public AttributeBase {
  protected:
    ENodeBCollector* collector_; // it has the cellCollector and the map <Ipue -> uecollector>

  /**
   * 
   * or std::map<ipv4, cellUeInfo>
   * I prefer the pointer to the list of users in the cell to manage better
//   * new/deleted users without the need of take care of them here
//   */
//    UeList* ueList_;
//    //Ecgi ecgi_;

    nlohmann::json toJsonCell() const; //should be private?
  /* data */


public:
  CellInfo();
  CellInfo(::omnetpp::cModule* eNodeB);
  virtual ~CellInfo();

  nlohmann::json toJson() const override;
//  nlohmann::json toJson(std::vector<Ipv4>& uesID) const;

};

#endif // _CELLINFO_H_

