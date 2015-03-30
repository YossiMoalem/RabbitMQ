#ifndef MY_CONNECTION_HANDLER_H
#define MY_CONNECTION_HANDLER_H

#include <future>

#include <amqpcpp.h>
#include "basicSocket.h"
#include "SmartBuffer.h"
#include "BlockingQueue.h"


namespace AMQP {

class RabbitMessageBase;

class AmqpConnectionDetails;

class MyConnectionHandler : public AMQP::ConnectionHandler
{
 public:
   typedef  std::future< bool > OperationSucceeded;
   typedef std::function<int( const AMQP::Message& )> OnMessageReveivedCB;
   typedef std::shared_ptr< std::promise< bool > > OperationSucceededSetter;

   MyConnectionHandler( OnMessageReveivedCB onMsgReceivedCB );

   virtual ~MyConnectionHandler();

   /**
    * Blocking untill connection is either established or failes
    **/
   bool login( const AmqpConnectionDetails & connectionParams );

   int startEventLoop();

   OperationSucceeded declareQueue( const std::string & queueName, bool durable = false, bool exclusive = false, bool autoDelete = false );

   /**
    * ExchangeType: as defined at amqpcpp/exchangetype.h
    **/
   OperationSucceeded declareExchange( const std::string & exchangeName, ExchangeType type = AMQP::fanout, bool durable = false );

   OperationSucceeded bindQueue( const std::string & exchangeName, const std::string & queueName, const std::string & routingKey);

   OperationSucceeded unBindQueue( const std::string & exchangeName, const std::string & queueName, const std::string & routingKey);

   OperationSucceeded publish( const std::string & exchangeName, const std::string & routingKey, const std::string & message );

   bool connected() const;

 protected:
   void doPublish( const std::string & exchangeName, const std::string & routingKey, const std::string & message, OperationSucceededSetter operationSucceeded );

   void doBindQueue( const std::string & exchangeName, const std::string & queueName, const std::string & routingKey,  OperationSucceededSetter operationSucceeded );

   void doUnBindQueue( const std::string & exchangeName, const std::string & queueName, const std::string & routingKey, OperationSucceededSetter operationSucceeded );
   virtual void onConnected( AMQP::Connection *connection );

   virtual void onData(AMQP::Connection *connection, const char *data, size_t size);

   virtual void onError(AMQP::Connection *connection, const char *message);

   virtual void onClosed(AMQP::Connection *connection);

 private:

   void handleResponse( );
   void handleQueue( );

 private:
   basicSocket                      _socket;
   AMQP::Connection*                _connection;
   AMQP::Channel *                  _channel;
   bool                             _connected = false;
   OnMessageReveivedCB              _onMsgReceivedBC;
   SmartBuffer                      _sb;
   BlockingQueue<RabbitMessageBase * >  _jobQueue;
};
} //namespace AMQP
#endif
