#ifndef AMQP_CLIENT_IMPLE_H
#define AMQP_CLIENT_IMPLE_H

#include <boost/noncopyable.hpp>
#include <memory>
#include <future>

#include <amqpcpp.h>
#include "AmqpSocket.h"

namespace AMQP {
class AmqpConnectionDetails;

class AMQPConnection : private AMQP::ConnectionHandler, boost::noncopyable 
{
 public:
   typedef std::shared_ptr< std::promise< bool > > OperationSucceededSetter;
   typedef std::function<int( const AMQP::Message& )> OnMessageReveivedCB;

   AMQPConnection( OnMessageReveivedCB onMsgReceivedCB );
   virtual ~AMQPConnection ();

   void doPublish( const std::string & exchangeName, 
           const std::string & routingKey, 
           const std::string & message, 
           OperationSucceededSetter operationSucceeded ) const;

   void doBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey,  
           OperationSucceededSetter operationSucceeded ) const;

   void doUnBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey, 
           OperationSucceededSetter operationSucceeded ) const;

   void handleInput( );
   /**
    * Blocking untill connection is either established or failes
    **/
   bool login( const AmqpConnectionDetails & connectionParams );

// protected:

   virtual void onConnected( AMQP::Connection *connection );

   virtual void onData(AMQP::Connection *connection, const char *data, size_t size);

   virtual void onError(AMQP::Connection *connection, const char *message);

   virtual void onClosed(AMQP::Connection *connection);

   std::future< bool > declareExchange( const std::string & exchangeName, 
           ExchangeType type = AMQP::fanout, 
           bool durable = false ) const ;

   std::future< bool > declareQueue( const std::string & queueName, 
           bool durable = false, 
           bool exclusive = false, 
           bool autoDelete = false ) const;

   std::future< bool > bindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey) const;

   std::future< bool > unBindQueue( const std::string & exchangeName,
           const std::string & queueName, 
           const std::string & routingKey) const;

   std::future< bool > publish( const std::string & exchangeName, 
           const std::string & routingKey, 
           const std::string & message ) const;

 private:
   AmqpSocket                       _socket;
   AMQP::Connection*                _connection;
   AMQP::Channel *                  _channel;
   bool                             _connected = false;
   OnMessageReveivedCB  _onMsgReceivedBC;
   SmartBuffer                      _incomingMessages;
   SmartBuffer                      _outgoingMessages;
};

} //namespace AMQP
#endif
