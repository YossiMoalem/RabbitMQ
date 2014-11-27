#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <sstream>
#include "simpleClient.h"

#define RABBIT_DEBUG(MSG) do{\
    std::stringstream ss;\
    ss<< MSG <<std::endl;\
    std::cerr << ss.str();\
}while(0);

enum class StopStatus
{
   Continue = 0,
   StopGracefull = 1,
   StopImmediate = 2
};
class clientMessage
{
    public:
    clientMessage()
    { };

    clientMessage(const std::string& i_text, 
            const std::string& i_destination ) :
        m_text(i_text),
        m_destination(i_destination)
    { };

    std::string m_text;
    std::string m_destination;
};
typedef clientMessage Protocol;

static const char* const ExchangeTypeStr[ (int)ExchangeType::Last ] = {"direct", "topic", "fanout"};
#endif
