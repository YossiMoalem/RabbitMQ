#ifndef BASIC_SOCKET_H
#define BASIC_SOCKET_H

#include <string>
#include <string.h>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>

#include "SmartBuffer.h"

namespace AMQP{

class AMQPSocket : boost::noncopyable
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
            std::cout <<"Faild getaddrinfo. Result was" << addrinfiResult <<std::endl;
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
                std::cerr<<"Error creating Socket " <<std::endl;
            } else {
                if( ::connect(_socketFd, 
                            currentAddress->ai_addr, 
                            currentAddress->ai_addrlen) < 0 )
                {
                    std::cerr <<"Error : Connect Failed "<< std::endl;
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
//            return false;
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

    //TODO: IF we read full buffer, and the socket is still ready
    //the event loop will not schedual another read!
    //we need to make sure we either:
    //read untill the socket has nothing to read from, or
    //we schedual another read manually in teh event loop.
    //the event loop will NOT take care of this!
    bool read( SmartBuffer & sbuffer)
    {
        const int buffSize = 2048;
        char buff[buffSize];
        bzero( buff, buffSize );
        ssize_t bytesRead = ::read( _socketFd, buff, buffSize);
        if( bytesRead < 0 )
        {
            if( errno != 11 /* not EWOULDBLOCK, EAGAIN */)
              std::cout << "ERROR RECEIVING!: errno = " <<errno <<std::endl;
          throw "Socket down?";
            return false;
        }
        if( bytesRead == 0 )
        {
          std::cout << "read 0 bytes." <<std::endl;
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

    int readFD() const 
    {
        return _socketFd;
    }

 private:
   int _socketFd;
};

} //namespace AMQP
#endif
