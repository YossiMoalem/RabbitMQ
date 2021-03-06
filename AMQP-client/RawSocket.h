#ifndef RAW_SOCKET_H
#define RAW_SOCKET_H

#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include "Debug.h"
#include "SmartBuffer.h"

namespace AMQP{

class RawSocket : boost::noncopyable
{
 public:

    bool connect(const std::string & address, unsigned int port )
    {
        std::string portStr = boost::lexical_cast< std::string > ( port );
        return connect( address, portStr );
    }

    bool connect(const std::string & address, const std::string & port )
    {
        assert (_socketFd == 0 );
        addrinfo hints;
        addrinfo * resolvedAddr;
        addrinfo * currentAddress;

        memset( & hints, 0, sizeof( hints ) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = 0 ; 
        hints.ai_flags = 0;
        hints.ai_protocol = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        int addrinfiResult = getaddrinfo( address.c_str(), port.c_str(), &hints, & resolvedAddr );
        if( addrinfiResult != 0 )
        {
            PRINT_DEBUG(DEBUG, "Failed getaddrinfo. Result was" << addrinfiResult);
            return false;
        }

        bool connected = false;
        for( currentAddress = resolvedAddr; 
                currentAddress != NULL && ! connected;
                currentAddress = currentAddress->ai_next )
        {
            _socketFd = socket( currentAddress->ai_family,
                    currentAddress->ai_socktype,
                    currentAddress->ai_protocol );

            if( _socketFd < 0 )
            {
                PRINT_DEBUG(DEBUG, "Error creating Socket. errno: " <<errno );
            } else {
                if( ::connect(_socketFd, 
                            currentAddress->ai_addr, 
                            currentAddress->ai_addrlen) < 0 )
                {
                    PRINT_DEBUG(DEBUG, "Error : Connect Failed");
                    ::close( _socketFd );
                }  else {
                    connected = true;
                }
            }
        }
        freeaddrinfo( resolvedAddr );
        return connected;
    }

    bool send( SmartBuffer & buffer)
    {
        assert (_socketFd > 0 );
        ssize_t bytesSent = ::send( _socketFd, buffer.data(), buffer.size(), MSG_NOSIGNAL);
        if (bytesSent < 0)
        {
            PRINT_DEBUG(DEBUG, "Send failed with errno: " <<errno);
            throw std::runtime_error( "Closed Socket" );
        }
        buffer.shrink( bytesSent );
        if ( ! buffer.empty() )
            return false;
        return true;
    }

    bool read( SmartBuffer & buffer)
    {
        assert (_socketFd > 0 );
        static constexpr  int buffSize = 4096;
        char buff[ buffSize ];
        bzero( buff, buffSize );
        ssize_t bytesRead = ::read( _socketFd, buff, buffSize);
        if( bytesRead < 0 )
        {
            if( errno != EWOULDBLOCK && errno !=  EAGAIN )
                PRINT_DEBUG(DEBUG, "ERROR RECEIVING!: errno = " <<errno);
          throw std::runtime_error( "Closed Socket" );
        }
        if( bytesRead == 0 )
        {
          PRINT_DEBUG(DEBUG, "read 0 bytes.");
          throw std::runtime_error( "Closed Socket" );
        }
        buffer.append(buff, bytesRead);
        return true;
    }

    void close()
    {
        ::close( _socketFd );
        _socketFd = 0;
    }

    int writeFD() const 
    {
        assert (_socketFd > 0 );
        return _socketFd;
    }

    int readFD() const 
    {
        assert (_socketFd > 0 );
        return _socketFd;
    }

 private:
   int _socketFd = 0;
};

} //namespace AMQP
#endif
