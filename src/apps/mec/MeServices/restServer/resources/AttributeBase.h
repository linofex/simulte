#ifndef _ATTRIBUTEBASE_H_
#define _ATTRIBUTEBASE_H_


#include "json.hpp"
#include <ctime>
#include <string>


class AttributeBase 
{
    public:
        AttributeBase();
        virtual ~AttributeBase();
        virtual nlohmann::json toJson() const = 0;
//        virtual void fromJson(nlohmann::json& json) = 0;

        static std::string toJson( const std::string& value );
        static std::string toJson( const std::time_t& value );
        static int32_t toJson( int32_t value );
        static int64_t toJson( int64_t value );
        static double toJson( double value );
        static bool toJson( bool value );
        static nlohmann::json toJson(AttributeBase& content );
};

#endif // _ATTRIBUTEBASE_H_