#ifndef AMQP_CLIENT_H
#define AMQP_CLIENT_H

#include "BlockingQueue.h"
#include <amqpcpp.h>

#include <boost/noncopyable.hpp>
#include <future>

//TODO: 
#include "RabbitOperation.h"

namespace AMQP {

class RabbitMessageBase;
class AMQPConnectionDetails;
class AMQPConnectionHandler;

class AMQPClient : private boost::noncopyable
{
 public:
   typedef std::function<int( const AMQP::Message& )> OnMessageReveivedCB;

   AMQPClient( OnMessageReveivedCB onMsgReceivedCB );

   ~AMQPClient();

   //TODO: should keep the connection details from start!
   //TODO: what if event loop is not started (say, did not manage to connect?)
   //wait for it, or, better indicate upstream that it is not connected.
   DeferedResult login( const AMQPConnectionDetails & connectionParams );

   //TODO: need some form of identication upwords that we are ready to contionue.
   int connect( const AMQPConnectionDetails & connectionParams );

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


 private:
   AMQPConnectionHandler *              _connectionHandler;
   mutable BlockingQueue<RabbitMessageBase * >  _jobQueue;

};
} //namespace AMQP
#endif
