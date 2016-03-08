#ifndef AMQP_CLIENT_H
#define AMQP_CLIENT_H

#include "RabbitJobHandler.h"
#include "RabbitJobQueue.h"
#include "Types.h"

#include <boost/noncopyable.hpp>

namespace AMQP {

class RabbitClient : private boost::noncopyable
{
 public:
   RabbitClient( OnMessageReceivedCB onMsgReceivedCB );

   /**
    * Establish connection to the server, no login yet
    * Initiate event loop
    *
    * If it failes, leaves the system unchanged
    * If it succeeds, need to call stop() to terminate the service
    **/
   bool init( const RabbitConnectionDetails & connectionParams );

   DeferredResult login() const;

   DeferredResult stop( bool immediate ) const;

   DeferredResult declareQueue( const std::string & queueName, 
           bool durable = false, 
           bool exclusive = false, 
           bool autoDelete = false ) const;

   DeferredResult removeQueue( const std::string & queueName ) const;

   /**
    * ExchangeType: as defined at amqpcpp/exchangetype.h
    **/
   DeferredResult declareExchange( const std::string & exchangeName, 
           ExchangeType type = AMQP::fanout, 
           bool durable = false ) const ;

   DeferredResult bindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey) const;

   DeferredResult unBindQueue( const std::string & exchangeName,
           const std::string & queueName, 
           const std::string & routingKey) const;

   DeferredResult publish( const std::string & exchangeName, 
           const std::string & routingKey, 
           const std::string & message ) const;

   bool connected() const;
   void waitForDisconnection() const;

 private:
   mutable RabbitJobQueue                       _jobQueue;
   mutable RabbitJobHandler                     _jobHandler;
   RabbitConnectionDetails                      _connectionParams;

};
} //namespace AMQP
#endif
