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
   //TODO: this is for the connection handler to be able to break from login wait
   //in case the event loop exits. Needs to be removed when login moved to event loop thread
   bool active() { return ! _stop ; } 

   void publish( const std::string & exchangeName, 
           const std::string & routingKey, 
           const std::string & message, 
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

   void bindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey,  
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

   void unBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey, 
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

   void login( const std::string & userName,
           const std::string & password,
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

   void declareExchange( const std::string & exchangeName, 
           ExchangeType exchangetype,
           bool durable,
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

    void declareQueue( const std::string & queueName, 
            bool durable, 
            bool exclusive, 
            bool autoDelete,
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

   void stop( bool immediate );

 private:
   void handleQueue( );
   void _resetTimeout( timeval & timeoutTimeval );
 private:
   bool                                     _stop = false;
   std::unique_ptr< AMQPConnectionHandler > _connectionHandlers;
   BlockingQueue<RabbitMessageBase * > *    _jobQueue;
};

} //namespace AMQP

#endif
