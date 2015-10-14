#ifndef AMQP_HEARTBEAT_H
#define AMQP_HEARTBEAT_H

#include <boost/noncopyable.hpp>
#include "RabbitConnection.h"

namespace AMQP{

class Heartbeat : boost::noncopyable
{
 public:
   Heartbeat( const RabbitConnection & dataConnection );
   void invalidate();
   bool send();
   void initialize();
   void reset ();

 private:
   RabbitConnection         _connection;
   bool                     _initialized = false;
   bool                     _initializeCalled = false;
   bool                     _heartbeatSent = false;
   std::string              _adminQueueName; 
   constexpr static auto _adminExchangeName = "admin" ;
   constexpr static auto _adminRoutingKey = "admin" ;
};

}//namespace AMQP
#endif
