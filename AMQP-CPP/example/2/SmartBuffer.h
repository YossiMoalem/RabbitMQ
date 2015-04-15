#ifndef SMART_BUFFER_H
#define SMART_BUFFER_H
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <stdint.h>             /* Definition of uint64_t */
#include <sys/eventfd.h>
#include <unistd.h>


class SmartBuffer
{
 public:
    friend std::ostream& operator <<(std::ostream& stream, const SmartBuffer& sb);

    SmartBuffer( ):
        _eventFD( eventfd( 0, 0 ) )
    {
    }

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

    char* data()
    {
        return _buffer.data();
    }

    char* shrink( unsigned int elementsToRemove )
    {
        elementsToRemove = std::min( elementsToRemove,(unsigned int)_buffer.size() );
        _buffer.erase ( _buffer.begin(), _buffer.begin()+elementsToRemove );
        ssize_t i = elementsToRemove;
        read( _eventFD, & i, sizeof( uint64_t ) );
        return _getBuffer();
    }

    void append( const char* data, unsigned int size )
    {
        _buffer.insert( _buffer.end(), data, data+size );
        uint64_t i = size;
        write( _eventFD, &i, sizeof( uint64_t ) );
    }

    void clear()
    {
        _buffer.clear();
        ssize_t i = _buffer.size();
        read( _eventFD, & i, sizeof( uint64_t ) );
    }

    int getFD() const
    {
        return _eventFD;
    }

 private:
    char* _getBuffer()
    {
        return _buffer.data()+_buffer.size();
    }

 private:
    std::vector<char>   _buffer;
    int                 _eventFD;

};

#endif