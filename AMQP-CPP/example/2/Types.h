#ifndef RABBIT_TYPES_H
#define RABBIT_TYPES_H

#include <memory>
#include <future>

namespace AMQP{
class Message;

using DeferedResult = std::future< bool >;
using DeferedResultSetter = std::shared_ptr< std::promise< bool > >;
using OnMessageReveivedCB = std::function<int( const AMQP::Message& )> ;
#define dummyResultSetter nullptr

struct RabbitConnectionDetails
{
 public:
   RabbitConnectionDetails()
   {}

   RabbitConnectionDetails(const std::string& i_userName,
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
