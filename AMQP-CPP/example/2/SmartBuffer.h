#ifndef SMART_BUFFER_H
#define SMART_BUFFER_H
#include <algorithm>
#include <iostream>
#include <vector>
#include <stdint.h>             /* Definition of uint64_t */
#include <sys/eventfd.h>
#include <unistd.h>
#include <boost/noncopyable.hpp>

class SmartBuffer : boost::noncopyable
{
 public:
    friend std::ostream& operator <<(std::ostream& stream, const SmartBuffer& sb);

    size_t empty()
    {
        return ( _buffer.size() == 0 );
    }

    size_t size()
    {
        return ( _buffer.size() );
    }

    size_t capacity()
    {
        return _buffer.capacity();
    }

    const char* data()
    {
        return _buffer.data();
    }

    size_t shrink( uint64_t elementsToRemove )
    {
        if ( elementsToRemove > 0 )
        {
            elementsToRemove = ( uint64_t )std::min( elementsToRemove, _buffer.size() );
            _buffer.erase ( _buffer.begin(), _buffer.begin() + elementsToRemove );
        }
        return size();
    }

    void append( const char* data, uint64_t size )
    {
        if( size > 0 )
        {
            _buffer.insert( _buffer.end(), data, data + size );
        }
    }

    void clear()
    {
        uint64_t i = _buffer.size();
        if ( i > 0 )
        {
            _buffer.clear();
        }
    }

 private:
    std::vector<char>   _buffer;

};

#endif
