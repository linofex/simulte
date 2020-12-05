#ifndef _ECGI_H_
#define _ECGI_H_

#include <string>

#include "apps/mec/MeServices/Resources/AttributeBase.h"
#include "../../RNIService/resources/Plmn.h"


class Ecgi : public AttributeBase {
  protected:
    std::string cellId_;
    Plmn plmn_;

    nlohmann::ordered_json toJsonCell() const; //should be private?

public:
  Ecgi();
  Ecgi(std::string& cellId);
  Ecgi(const mec::Ecgi ecgi);

  Ecgi(std::string& cellId, Plmn& plmn);
  
  virtual ~Ecgi();
  
  void setCellId(const std::string& cellId);
  void setEcgi(const mec::Ecgi& ecgi);

  std::string getCellId() const;
  Plmn getPlmn() const;

  nlohmann::ordered_json toJson() const override;

};

#endif // _ECGI_H_

