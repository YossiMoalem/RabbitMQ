#ifndef AMQP_CONNECTION_H
#define AMQP_CONNECTION_H
//

#include <myConnectionHandler.h>
#include "Types.h"

#include <thread>

class AMQPConnection
{
 public:
   AMQPConnection( const ConnectionDetails & connectionDetails,
           const std::string & exchangeName ,
           const std::string & queueName,
           const std::string & routingKey,
           AMQP::MyConnectionHandler::OnMessageReveivedCB i_onMessageReceiveCB ) :
       _connectionHandler( [ i_onMessageReceiveCB ] 
               ( const AMQP::Message & message ) 
               { return i_onMessageReceiveCB( message ); } ),
       _connectionDetails( connectionDetails ),
       _stop( false ),
       _exchangeName( exchangeName ),
       _queueName( queueName ),
       _routingKey( routingKey )
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

       _connectionHandler.bindQueue( _exchangeName, _queueName, _routingKey );
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
   ConnectionDetails            _connectionDetails;
   bool                         _stop;
   std::thread                  _eventLoopThread;
   std::string                  _exchangeName;
   std::string                  _queueName;
   std::string                  _routingKey;
};

#endif
