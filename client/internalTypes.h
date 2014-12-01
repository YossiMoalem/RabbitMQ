#ifndef INTERNAL_TYPES_H
#define INTERNAL_TYPES_H

#define RABBIT_DEBUG(MSG) do{\
    std::stringstream ss;\
    ss<< MSG <<std::endl;\
    std::cerr << ss.str();\
}while(0);

#include <functional>

enum class DeliveryType;

enum class RunStatus 
{
     Continue = 0,
     StopGracefull = 1,
     StopImmediate = 2
};

enum class MessageType
{
    Post,
    Bind,
    Unbind
};

typedef std::function<int (std::string o_messageText, std::string o_senderID, DeliveryType o_deliverySTatus )> CallbackType;

#endif
