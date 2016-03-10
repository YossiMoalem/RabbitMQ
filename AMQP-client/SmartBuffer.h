#ifndef SMART_BUFFER_H
#define SMART_BUFFER_H

#include <iostream>
#include <vector>
#include <sys/eventfd.h>
#include <unistd.h>
#include <boost/noncopyable.hpp>

namespace AMQP {

class SmartBuffer : boost::noncopyable
{
 public:
    friend std::ostream& operator <<(std::ostream& stream, const SmartBuffer& sb);

    size_t empty() const
    {
        return ( _buffer.size() == 0 );
    }

    size_t size() const
    {
        return ( _buffer.size() );
    }

    size_t capacity() const
    {
        return _buffer.capacity();
    }

    const char * data()
    {
        return _buffer.data();
    }

    size_t shrink( size_t consumedElements )
    {
        consumedElements = std::min( consumedElements, size() );
        _buffer.erase ( _buffer.begin(), _buffer.begin() + consumedElements );
        return size();
    }

    void append( const char* data, size_t size )
    {
        _buffer.insert( _buffer.end(), data, data + size );
    }

    void clear()
    {
        _buffer.clear();
    }

 private:
    std::vector< char >   _buffer;

};
} //namespace AMQP
#endif
