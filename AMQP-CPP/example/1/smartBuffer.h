#include <iostream>
#include <vector>
#include <string>

class smartBuffer
{
 public:
   smartBuffer( unsigned int length )
    {
        _buffer = std::vector<char>(length);
//        _used = 0;
    }

    size_t size()
    {
        return ( _buffer.size() );
    }

//    void increaseUsedSize(int change)
//    {
//          //TODO: if new size is bigger than capacity, we should alert...
//        _used += change;
//    }

    size_t capacity()
    {
        return _buffer.capacity();
    }

    char* data()
    {
        return _buffer.data();
    }

    char* extend()
    {
        _buffer.reserve( _buffer.capacity()*2 );
        return getBuffer();
    }

    char* shrink(unsigned int elementsToRemove)
    {
        //TODO: make sure not to shrink more elements than what we already have
        _buffer.erase ( _buffer.begin(), _buffer.begin()+elementsToRemove );
        return getBuffer();
    }

    char* getBuffer()
    {
        return _buffer.data()+_buffer.size();
    }

 private:
   std::vector<char>    _buffer;
//   unsigned int         _used;
};