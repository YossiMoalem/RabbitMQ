#ifndef BASIC_SOCKET_H
#define BASIC_SOCKET_H

#include <string>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <unistd.h>
#include <smartBuffer.h>

class basicSocket
{
 public:
   basicSocket( const std::string & IP, unsigned int port ) :
       _IP( IP ),
       _port( port ),
       _sb( 0 )
    {}

    bool connect()
    {
        _socketFd = socket( AF_INET, SOCK_STREAM, 0);
        if( _socketFd < 0 )
        {
            std::cerr<<"Error creating Socket " <<std::endl;
            return false;
        }

        sockaddr_in serv_addr = { 0 };
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons( _port ); 
        if( inet_pton(AF_INET, _IP.c_str(), &serv_addr.sin_addr) <= 0 )
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

    void send( const char* data, size_t size )
    {
        //TODO: loop until we send all the buffer
        std::cout <<"sending: ( " << size << " ):";
        //std::cout.write(data, size );
        std::cout <<std::endl;
        ssize_t result = ::send( _socketFd, data, size, MSG_NOSIGNAL);
        if ( result < 0 )
        {
            //TODO: raise/return error according to design me and yos decide
            std::cout <<"we need to reconnecttttttttt: ( " << size << " ):";
        }

    }

    ssize_t read( char buffer[], size_t bufferSize )
    {
        bzero( buffer, bufferSize );
        ssize_t size = ::read( _socketFd, buffer, bufferSize);
//        ssize_t size = ::read( _socketFd, _sb.getBuffer(), _sb.capacity() );
        _sb.addToBuffer(size, buffer);
//        _sb.print();
        std::cout <<"Got: " << size <<" Bytes: ";
        //std::cout.write( buff, size );
        std::cout <<std::endl;
        return size;
    }

 private:
   const std::string    _IP;
   unsigned int         _port;
   int                  _socketFd;
   smartBuffer          _sb;
};

#endif
