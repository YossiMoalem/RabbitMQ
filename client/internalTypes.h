#ifndef INTERNAL_TYPES_H
#define INTERNAL_TYPES_H

#include "Types.h"


enum class DeliveryType;
class RabbitMessageBase;
template<typename Data>
class BlockingQueue;

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


static const char* const ExchangeTypeStr[ (int)ExchangeType::Last ] = {"direct", "topic", "fanout"};

typedef BlockingQueue<RabbitMessageBase*> MessageQueue;
#endif
