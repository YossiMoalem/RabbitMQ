#ifndef BASIC_SOCKET_H
#define BASIC_SOCKET_H

#include <sys/fcntl.h>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#include "SmartBuffer.h"

namespace AMQP{

class AmqpSocket
{
 public:

    bool connect(const std::string & IP, unsigned int port )
    {
        _socketFd = socket( AF_INET, SOCK_STREAM, 0);
        if( _socketFd < 0 )
        {
            std::cerr<<"Error creating Socket " <<std::endl;
            return false;
        }

        struct timeval tv;
        tv.tv_sec = 1 ; 
        tv.tv_usec = 0;

        setsockopt(_socketFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
// TODO: fix the error in the next line because we want the socket to be non blocking!
//        fcntl(_socketFd, F_SETFL, O_NONBLOCK);  // set to non-blocking

        sockaddr_in serv_addr = { 0 };
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons( port ); 
        if( inet_pton(AF_INET, IP.c_str(), &serv_addr.sin_addr) <= 0 )
        {
            std::cerr<<"inet_pton error occured" <<std::endl;
            return false;
        }

        if( ::connect(_socketFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            std::cerr <<"Error : Connect Failed "<< std::endl;
            return false;
        } 
        return true;
    }

    bool send( SmartBuffer & sbuffer)
    {
        ssize_t bytesSent = ::send( _socketFd, sbuffer.data(), sbuffer.size(), MSG_NOSIGNAL);
        if (bytesSent < 0)
        {
            return false;
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
              std::cout <<"ERROR RECEIVING!: errno = " <<errno <<std::endl;
            return false;
        }
        if( bytesRead == 0 )
        {
          return false;
        }
        sbuffer.append(buff, bytesRead);
        return true;
    }

    int readFD() const 
    {
        return _socketFd;
    }

 private:
   int                  _socketFd;
};

} //namespace AMQP
#endif
