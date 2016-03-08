#ifndef AMQP_HEARTBEAT_H
#define AMQP_HEARTBEAT_H

#include "RabbitConnection.h"

namespace AMQP{

class Heartbeat : boost::noncopyable
{
 public:
   Heartbeat( const RabbitConnection & dataConnection );
   Heartbeat( const Heartbeat & ) = delete;
   Heartbeat & operator = (const Heartbeat & ) = delete;
   void invalidate();
   bool send();
   void reset ();

 private:
   void _initialize();
   void _createAdminQueue();
   void _sendHeartbeat();

 private:
   RabbitConnection         _connection;
   bool                     _heartbeatSent;
   bool                     _initialized;
   std::string              _adminQueueName; 
   constexpr static auto   _adminExchangeName = "admin" ;
   constexpr static auto   _adminRoutingKey = "admin" ;
};

}//namespace AMQP
#endif
