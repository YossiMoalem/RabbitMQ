#ifndef SMART_BUFFER_H
#define SMART_BUFFER_H
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

class SmartBuffer
{
 public:
    friend std::ostream& operator <<(std::ostream& stream, const SmartBuffer& sb);

    SmartBuffer( )
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
        return _getBuffer();
    }

    void append( const char* data, unsigned int size )
    {
        _buffer.insert( _buffer.end(), data, data+size );
    }

    void clear()
    {
        _buffer.clear();
    }

 private:
    char* _getBuffer()
    {
        return _buffer.data()+_buffer.size();
    }

 private:
    std::vector<char>    _buffer;

};

#endif