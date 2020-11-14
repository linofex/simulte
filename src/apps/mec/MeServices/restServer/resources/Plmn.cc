#include "Plmn.h"

Plmn::Plmn()
{
    // test mcc and mnc
    mcc_ = "001";
    mnc_ = "01";
}


Plmn::Plmn(const std::string& mcc, const std::string& mnc)
{
    setMcc(mcc);
    setMnc(mnc);
}

Plmn::~Plmn(){}

void Plmn::setMcc(const std::string& mcc)
{
    mcc_ = mcc;
}

void Plmn::setMnc(const std::string& mnc)
{
    mnc_ = mnc;
}

std::string Plmn::getMcc() const
{
    return mcc_;
}
std::string Plmn::getMnc() const
{
    return mnc_;
}

nlohmann::json Plmn::toJson() const
{
    nlohmann::json val = nlohmann::json::object();
    val["mcc"] = mcc_;
    val["mnc"] = mnc_;

    return val;

}


