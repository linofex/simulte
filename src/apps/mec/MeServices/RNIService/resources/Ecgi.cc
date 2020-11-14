#include "../../RNIService/resources/Ecgi.h"


Ecgi::Ecgi():plmn_()
{
    cellId_ = "";
}

Ecgi::Ecgi(std::string& cellId):plmn_()
{
    setCellId(cellId);
}

Ecgi::Ecgi(std::string& cellId, Plmn& plmn):plmn_(plmn)
{
    setCellId(cellId);
}

Ecgi::~Ecgi(){}

void Ecgi::setCellId(const std::string& cellId)
{
    cellId_ = cellId;
}

void Ecgi::setEcgi(const mec::Ecgi& ecgi)
{
    setCellId(ecgi.cellId);
    plmn_.setMnc(ecgi.plmn.mnc);
    plmn_.setMcc(ecgi.plmn.mcc);
}

std::string Ecgi::getCellId() const
{
    return cellId_;
}

Plmn Ecgi::getPlmn() const
{
    return plmn_;
}


nlohmann::ordered_json Ecgi::toJson() const
{
    nlohmann::ordered_json val;
    val["cellId"] = cellId_;
    val["plmn"] = plmn_.toJson();
    

    return val;
}

