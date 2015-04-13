#ifndef AMQP_CONNECTION_DETAILS_H
#define AMQP_CONNECTION_DETAILS_H

#include <string>

namespace AMQP {

struct AMQPConnectionDetails
{
 public:
   AMQPConnectionDetails(const std::string& i_userName,
           const std::string& i_password,
           const std::string& i_host,
           int port) :
       _userName( i_userName), 
       _password( i_password), 
       _host( i_host),
       _port( port )
    {}

   std::string  _userName;
   std::string  _password;
   std::string  _host;
   int          _port;
};
} //namespace AMQP
#endif
