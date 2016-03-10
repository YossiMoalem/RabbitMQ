#ifndef RABBIT_TYPES_H
#define RABBIT_TYPES_H

#include <memory>
#include <future>

namespace AMQP{
class Message;

using DeferredResult = std::future< bool >;
using DeferredResultSetter = std::shared_ptr< std::promise< bool > >;
using OnMessageReceivedCB = std::function<int( const AMQP::Message& )> ;
#define dummyResultSetter nullptr

struct RabbitConnectionDetails
{
 public:
   RabbitConnectionDetails()
   {}

   RabbitConnectionDetails(const std::string& userName,
           const std::string& password,
           const std::string& host,
           int port) :
       userName( userName), 
       password( password), 
       host( host),
       port( port )
    {}

   std::string  userName;
   std::string  password;
   std::string  host;
   int          port;
};

} //namespace AMQP
#endif
