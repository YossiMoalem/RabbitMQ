#include "SmartBuffer.h"
#include <string>

namespace AMQP{
std::ostream & operator<<(std::ostream &os, const SmartBuffer& sb)
{
    // std::for_each (sb._buffer.begin(), sb._buffer.end(), [ &os ] (char i) { os<<i; });
    os <<"buffer size: " << sb._buffer.size() <<std::endl ;
    os <<"buffer capacity: " << sb._buffer.capacity() <<std::endl ;
    return os;
}
}
