#ifndef COMMON_H
#define COMMON_H

#include "SynchronizedQueue.h"
#include <iostream>
#include <sstream>

#define RABBIT_DEBUG(MSG) do{\
    std::stringstream ss;\
    ss<< MSG <<std::endl;\
    std::cerr << ss.str();\
}while(0);

enum StopStatus
{
    SS_Continue = 0,
    SS_StopGracefull = 1,
    SS_StopImmediate = 2
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

typedef SynchronizedQueue<Protocol> MessageQueue;
#endif
