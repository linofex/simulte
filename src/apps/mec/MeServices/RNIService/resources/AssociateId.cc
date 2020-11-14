
#include "AssociateId.h"

AssociateId::AssociateId()
{
    type_ = "";
    value_ = "";
}

AssociateId::AssociateId(std::string& type,std::string& value)
{
    setType(type);
    setValue(value);
}

AssociateId::AssociateId(mec::AssociateId& associateId)
{
    setType(associateId.type);
    setValue(associateId.value);
}


AssociateId::~AssociateId()
{
}

nlohmann::ordered_json AssociateId::toJson() const
{
    nlohmann::ordered_json val;

    val["type"] = type_;
    val["value"] = value_;
    return val;
}


void AssociateId::setAssociateId(const mec::AssociateId& associateId)
{
    type_  = associateId.type;
    value_ = associateId.value;
}

std::string AssociateId::getType() const
{
    return type_;
}
void AssociateId::setType(std::string value)
{
    type_ = value;
    
}
std::string AssociateId::getValue() const
{
    return value_;
}
void AssociateId::setValue(std::string value)
{
    value_ = value;
    
}


