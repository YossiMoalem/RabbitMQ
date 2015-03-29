#ifndef SMART_BUFFER_H
#define SMART_BUFFER_H
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

class SmartBuffer
{
 public:
    SmartBuffer( unsigned int length=0 )
    {
        _buffer = std::vector<char>(length);
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

    char* shrink(unsigned int elementsToRemove)
    {
        elementsToRemove = std::min( elementsToRemove,(unsigned int)_buffer.size() );
        _buffer.erase ( _buffer.begin(), _buffer.begin()+elementsToRemove );
        return getBuffer();
    }

    void addToBuffer(unsigned int size, char* data)
    {
        _buffer.insert( _buffer.end(), data, data+size );
    }

    void print()
    {
        for( std::vector<char>::const_iterator i = _buffer.begin(); i != _buffer.end(); ++i)
            std::cout << *i;
        // std::cout <<"SIZE: " << _buffer.size() <<std::endl ;
        // std::cout <<"CAPACITY: " << _buffer.capacity() <<std::endl ;
    }

    char* getBuffer()
    {
        return _buffer.data()+_buffer.size();
    }

    void clear()
    {
        _buffer.clear();
    }

 private:
    std::vector<char>    _buffer;

//    char* _extend()
//    {
//        _buffer.reserve( _buffer.capacity()*2 );
//        return getBuffer();
//    }
};

#endif