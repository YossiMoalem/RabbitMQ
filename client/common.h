#ifndef COMMON_H
#define COMMON_H

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

enum class DeliveryType
{
    Unicast,
    Multicast
};

enum class MessageType
{
    Post,
    Bind,
    Unbind
};


static const char* const ExchangeTypeStr[ (int)ExchangeType::Last ] = {"direct", "topic", "fanout"};
#endif
