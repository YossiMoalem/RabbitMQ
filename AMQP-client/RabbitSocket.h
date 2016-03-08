#ifndef RABBIT_SOCKET_H
#define RABBIT_SOCKET_H

#include "RawSocket.h"
#include <assert.h>

namespace AMQP{

class RabbitSocket
{

 public:

   void shrink( size_t size )
   {
       _incomingBuffer.shrink( size );
   }

   bool connect(const std::string & address, unsigned int port )
   {
       bool connected = _socket.connect( address, port );
       if ( connected )
       {
           _readFD = _socket.readFD();
           _writeFD = _socket.writeFD();
           assert( _readFD > 0 );
           assert( _writeFD > 0 );
       }
       return connected;
   }

   bool connect(const std::string & address, const std::string & port )
   {
       return _socket.connect( address, port );
   }
   bool read( const char * & data, size_t & size )
   {
       bool succeeded = _socket.read( _incomingBuffer );
       data = _incomingBuffer.data();
       size = _incomingBuffer.size();
       return succeeded;
   }

   void clear()
   {
       _incomingBuffer.clear();
       _outgoingBuffer.clear();
   }
   bool pendingSend()
   { return ! _outgoingBuffer.empty(); }

   unsigned int outgoingBufferSize() const
   { return _outgoingBuffer.size(); }

   bool handleOutput()
   {
       assert( pendingSend() );
       return _socket.send( _outgoingBuffer);
   }
   void send( const char *data, size_t size)
   { _outgoingBuffer.append( data, size ); }

   void close()
   { _socket.close(); }

   int readFD() const noexcept
   { assert( _readFD > 0 ); return _readFD; }

   int writeFD() const 
   { assert( _writeFD > 0 ); return _writeFD; }

 private:
   int                              _readFD = 0;
   int                              _writeFD = 0;
   RawSocket                        _socket;
   SmartBuffer                      _incomingBuffer;
   SmartBuffer                      _outgoingBuffer;
};
} //namespace AMQP
#endif
