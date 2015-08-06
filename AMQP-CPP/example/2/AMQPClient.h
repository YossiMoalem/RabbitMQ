#ifndef AMQP_CLIENT_H
#define AMQP_CLIENT_H

#include <amqpcpp.h>
#include "RabbitJobManager.h"
#include "AMQPConnectionDetails.h"
#include "ResultCodes.h"

#include <boost/noncopyable.hpp>
#include <future>

namespace AMQP {

class RabbitMessageBase;
class AMQPConnectionHandler;

class AMQPClient : private boost::noncopyable
{
 public:
   typedef std::function<int( const AMQP::Message& )> OnMessageReveivedCB;

   AMQPClient( OnMessageReveivedCB onMsgReceivedCB );


   /**
    * Establish connection to the server, no login yet
    * Initiate event loop
    *
    * If it failes, leaves the system unchanged
    * If it secceeds, need to call stop() to terminate the service
    **/
   bool init( const AMQPConnectionDetails & connectionParams );

   DeferedResult login() const;

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
