#ifndef AMQP_CLIENT_H
#define AMQP_CLIENT_H

#include "BlockingQueue.h"
#include "AMQPConnection.h"

#include <amqpcpp.h>

#include <boost/noncopyable.hpp>
#include <future>

namespace AMQP {

class RabbitMessageBase;
class AmqpConnectionDetails;

class AMQPClient : private boost::noncopyable
{
 public:

   AMQPClient( AMQPConnection::OnMessageReveivedCB onMsgReceivedCB );

   ~AMQPClient();

   /**
    * Blocking untill connection is either established or failes
    **/
   bool login( const AmqpConnectionDetails & connectionParams );

   int startEventLoop();

   std::future< bool > declareQueue( const std::string & queueName, 
           bool durable = false, 
           bool exclusive = false, 
           bool autoDelete = false ) const;

   /**
    * ExchangeType: as defined at amqpcpp/exchangetype.h
    **/
   std::future< bool > declareExchange( const std::string & exchangeName, 
           ExchangeType type = AMQP::fanout, 
           bool durable = false ) const ;

   std::future< bool > bindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey) const;

   std::future< bool > unBindQueue( const std::string & exchangeName,
           const std::string & queueName, 
           const std::string & routingKey) const;

   std::future< bool > publish( const std::string & exchangeName, 
           const std::string & routingKey, 
           const std::string & message ) const;

   bool connected() const;

   private:
   void handleQueue( );

 private:
   AMQPConnection *     _AMQPConnection;
   mutable BlockingQueue<RabbitMessageBase * >  _jobQueue;

};
} //namespace AMQP
#endif
