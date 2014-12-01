#ifndef COMMON_H
#define COMMON_H

#define RABBIT_DEBUG(MSG) do{\
    std::stringstream ss;\
    ss<< MSG <<std::endl;\
    std::cerr << ss.str();\
}while(0);

#include <functional>
class AMQPMessage;

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

typedef std::function<int (std::string o_messageText, std::string o_senderID)> CallbackType;

#endif
