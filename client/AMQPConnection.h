#ifndef AMQP_CONNECTION_H
#define AMQP_CONNECTION_H

#include <myConnectionHandler.h>
#include "Types.h"
#include "ConnectionDetails.h"

class AMQPConnection
{
 public:
   AMQPConnection( const ConnectionDetails & connectionDetails,
           const std::string & exchangeName ,
           const std::string & queueName,
           const std::string & routingKey,
           AMQP::MyConnectionHandler::OnMessageReveivedCB i_onMessageReceiveCB );

   ReturnStatus start();

   ReturnStatus stop( bool immediate );

   void publish( const std::string & exchangeName, 
           const std::string & routingKey,
           const std::string & message ) const;

   ReturnStatus bind( const std::string & exchangeName,
           const std::string & queueName,
           const std::string routingKey) const;

   ReturnStatus unBind( const std::string & exchangeName, 
           const std::string & queueName,
           const std::string routingKey) const;

   bool connected() const;

 private:
   AMQP::MyConnectionHandler    _connectionHandler;
   ConnectionDetails            _connectionDetails;
   bool                         _stop;
   std::thread                  _eventLoopThread;
   std::string                  _exchangeName;
   std::string                  _queueName;
   std::string                  _routingKey;
};
#endif
