#include <iostream>
#include <vector>
#include <string>

class smartBuffer
{
 public:
    smartBuffer( unsigned int length )
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
        //TODO: make sure not to shrink more elements than what we already have
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

 private:
    std::vector<char>    _buffer;

//    char* _extend()
//    {
//        _buffer.reserve( _buffer.capacity()*2 );
//        return getBuffer();
//    }
};