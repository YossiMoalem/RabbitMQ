#ifndef AMQP_CONNECTION_H
#define AMQP_CONNECTION_H
//
// TODO: in macros case - once is enogh
#define UNICAST_PREFIX "ALL:"
#define MULTICAST_SUFFIX ":ALL"

#include <myConnectionHandler.h>
#include "Types.h"

#include <thread>

class AMQPConnection
{
 public:
   AMQPConnection( const ConnectionDetails & connectionDetails,
           const std::string & exchangeName ,
           const std::string & queueName,
          AMQP::MyConnectionHandler::OnMessageReveivedCB i_onMessageReceiveCB ) :
       _connectionHandler( [ i_onMessageReceiveCB ] ( const AMQP::Message & message ) { return i_onMessageReceiveCB( message ); } ),
       _connectionDetails( connectionDetails ),
       _stop( false ),
       _exchangeName( exchangeName ),
       _queueName( queueName )
       {}

   ReturnStatus start()
   {
       //TODO: handle disconnection:
       //1. when the thread exits - get new host, 
       //1.1 login
       //1.2 re-start event loop
       //1.3 rebind
       _stop = false;
       AMQP::AmqpConnectionDetails connectionDetails = _connectionDetails.getFirstHost();
       _connectionHandler.login( connectionDetails );
       _eventLoopThread = std::thread( std::bind( &AMQP::MyConnectionHandler::startEventLoop, &_connectionHandler ) );
       _connectionHandler.declareExchange( _exchangeName, AMQP::topic );
       _connectionHandler.declareQueue( _queueName );
       //TODO: WAIT! check retvals!

       //TODO: routing key creation should be in one place!
       std::string routingKey = UNICAST_PREFIX + _queueName;
       _connectionHandler.bindQueue( _exchangeName, _queueName, routingKey );
       //TODO: WAIT! check retvals!

       return ReturnStatus::Ok;
   }

   ReturnStatus stop( bool immediate )
   {
       _stop = true;
       //2. push stop message to teh connection handler( immediate ) ? top : end 
       _eventLoopThread.join();
       return ReturnStatus::Ok;
   }

   void publish( const std::string & exchangeName, 
           const std::string & routingKey,
           const std::string & message ) const
   {
       _connectionHandler.publish( _exchangeName, routingKey, message );
   }

   ReturnStatus bind( const std::string & exchangeName,
           const std::string & queueName,
           const std::string routingKey) const
   {
       _connectionHandler.bindQueue( exchangeName, queueName, routingKey );
       return ReturnStatus::Ok;
   }

   ReturnStatus unBind( const std::string & exchangeName, 
           const std::string & queueName,
           const std::string routingKey) const
   {
       _connectionHandler.unBindQueue( exchangeName, queueName, routingKey );
       return ReturnStatus::Ok;
   }

   bool connected() const
   {
       //implement
       return true;
   }

 private:
   AMQP::MyConnectionHandler    _connectionHandler;
   ConnectionDetails      _connectionDetails;
   bool                         _stop;
   std::thread              _eventLoopThread;
   std::string              _exchangeName;
   //Queue Name is also routing key for (self). Think of something....
   std::string              _queueName;
};

#endif
