#ifndef AMQP_EVENT_LOOP
#define AMQP_EVENT_LOOP

#include <functional>
#include <memory>

#include "RabbitOperation.h"

namespace AMQP {

template < typename T >
    class BlockingQueue;
class RabbitMessageBase;

class AMQPConnectionHandler;
class Message;

class AMQPEventLoop
{
 public:
   AMQPEventLoop(  std::function<int( const AMQP::Message& )> onMsgReceivedCB, 
           BlockingQueue< RabbitMessageBase * > * jobQueue ) ;
   int start();

   //TODO: this is temporary WA untill I'll move all actions to work via the eventloop.
   //till then, client sends commands to the broker, so it needs the connection handler.
   //this needs to be removed!!!!!
   AMQPConnectionHandler* connectionHandler() { return _connectionHandlers.get(); } 

   void publish( const std::string & exchangeName, 
           const std::string & routingKey, 
           const std::string & message, 
           RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const;

   void bindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey,  
           RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const;

   void unBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey, 
           RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const;

   void stop( bool immediate );

 private:
   void handleQueue( );
 private:
   bool                                     _stop = false;
   std::unique_ptr< AMQPConnectionHandler > _connectionHandlers;
   BlockingQueue<RabbitMessageBase * > *    _jobQueue;
};

} //namespace AMQP

#endif
