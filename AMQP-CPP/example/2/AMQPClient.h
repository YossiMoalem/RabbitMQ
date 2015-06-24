#ifndef AMQP_CLIENT_H
#define AMQP_CLIENT_H

#include <amqpcpp.h>
#include "RabbitJobManager.h"
#include "AMQPConnectionDetails.h"

#include <boost/noncopyable.hpp>
#include <future>

//TODO: 
#include "RabbitOperation.h"

namespace AMQP {

class RabbitMessageBase;
class AMQPConnectionHandler;

class AMQPClient : private boost::noncopyable
{
 public:
   typedef std::function<int( const AMQP::Message& )> OnMessageReveivedCB;

   AMQPClient( OnMessageReveivedCB onMsgReceivedCB );

   DeferedResult login();

   bool init( const AMQPConnectionDetails & connectionParams );

   DeferedResult stop( bool immediate );

   DeferedResult declareQueue( const std::string & queueName, 
           bool durable = false, 
           bool exclusive = false, 
           bool autoDelete = false ) const;

   /**
    * ExchangeType: as defined at amqpcpp/exchangetype.h
    **/
   DeferedResult declareExchange( const std::string & exchangeName, 
           ExchangeType type = AMQP::fanout, 
           bool durable = false ) const ;

   DeferedResult bindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey) const;

   DeferedResult unBindQueue( const std::string & exchangeName,
           const std::string & queueName, 
           const std::string & routingKey) const;

   DeferedResult publish( const std::string & exchangeName, 
           const std::string & routingKey, 
           const std::string & message ) const;

   bool connected() const;
   void waitForDisconnection();


 private:
   mutable RabbitJobManager                     _jobManager;
   AMQPConnectionDetails                        _connectionParams;

};
} //namespace AMQP
#endif
