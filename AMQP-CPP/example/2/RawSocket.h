#ifndef RAW_SOCKET_H
#define RAW_SOCKET_H

#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
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
                PRINT_DEBUG(DEBUG, "Error creating Socket");
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

    bool send( SmartBuffer & sbuffer)
    {
        ssize_t bytesSent = ::send( _socketFd, sbuffer.data(), sbuffer.size(), MSG_NOSIGNAL);
        if (bytesSent < 0)
        {
            PRINT_DEBUG(DEBUG, "Send failed with errno: " <<errno);
            //TODO: throw a real AMQPException
            throw "Socket down?";
        }
        else
        {
            sbuffer.shrink( bytesSent );
            if ( !sbuffer.empty() )
                return false;
        }
        return true;
    }

    bool read( SmartBuffer & sbuffer)
    {
        const int buffSize = 4096;
        char buff[buffSize];
        bzero( buff, buffSize );
        ssize_t bytesRead = ::read( _socketFd, buff, buffSize);
        if( bytesRead < 0 )
        {
            if( errno != 11 /* not EWOULDBLOCK, EAGAIN */)
              PRINT_DEBUG(DEBUG, "ERROR RECEIVING!: errno = " <<errno);
          throw "Socket down?";
            return false;
        }
        if( bytesRead == 0 )
        {
          PRINT_DEBUG(DEBUG, "read 0 bytes.");
//          return false;
          //TODO: throw a real AMQPException
          throw "Socket down?";
        }
        sbuffer.append(buff, bytesRead);
        return true;
    }

    void close()
    {
        ::close( _socketFd );
    }

    int writeFD() const 
    {
        return _socketFd;
    }

    int readFD() const 
    {
        return _socketFd;
    }

 private:
   int _socketFd;
};

} //namespace AMQP
#endif
