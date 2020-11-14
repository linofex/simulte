#ifndef _ATTRIBUTEBASE_H_
#define _ATTRIBUTEBASE_H_


#include <ctime>
#include <string>

#include "../../RNIService/resources/json.hpp"
#include "common/LteCommon.h"


class AttributeBase 
{
    public:
        AttributeBase();
        virtual ~AttributeBase();
        virtual nlohmann::ordered_json toJson() const = 0;
//        virtual void fromJson(nlohmann::ordered_json& json) = 0;

        static std::string toJson( const std::string& value );
        static std::string toJson( const std::time_t& value );
        static int32_t toJson( int32_t value );
        static int64_t toJson( int64_t value );
        static double toJson( double value );
        static bool toJson( bool value );
        static nlohmann::ordered_json toJson(AttributeBase& content );
};

#endif // _ATTRIBUTEBASE_H_
