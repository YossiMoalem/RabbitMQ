#ifndef TYPES_H
#define TYPES_H

#include <functional>
#include <sstream> 

//This is here so that tester can use it...
#define RABBIT_DEBUG(MSG) do{\
    std::stringstream ss;\
    ss<< MSG <<std::endl;\
    std::cerr << ss.str();\
}while(0);

enum class DeliveryType
{
    Unicast,
    Multicast
};

enum class ReturnStatus
{
    Ok,
    ClientDisconnected,
    ClientSuttingDown,
    OperationFailed
};

typedef std::function<int (	std::string		o_senderID,
              std::string   o_destination,
							DeliveryType	o_deliveryType,
              std::string		o_messageText )> CallbackType;

#endif
