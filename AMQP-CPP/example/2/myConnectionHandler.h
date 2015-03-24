#ifndef MY_CONNECTION_HANDLER_H
#define MY_CONNECTION_HANDLER_H

#include <amqpcpp.h>

#include "basicSocket.h"


class AmqpConnectionDetails;

typedef std::function<int( const AMQP::Message& )> CB;

class MyConnectionHandler : public AMQP::ConnectionHandler
{
 public:
   MyConnectionHandler( CB onMsgReceivedCB );
   virtual ~MyConnectionHandler();


   bool login( const AmqpConnectionDetails & connectionParams );

   void declareQueue( const char * queueName );

   void declareExchange( const char * exchangeName );

   void bindQueueToExchange( const char * routingKey);

   void unbindQueueToExchange( const char * routingKey);

   void receiveMessage();

   void publish( const char* routingKey, const char* message );

 protected:
   virtual void onConnected( AMQP::Connection *connection );

   virtual void onData(AMQP::Connection *connection, const char *data, size_t size);

   virtual void onError(AMQP::Connection *connection, const char *message);

   virtual void onClosed(AMQP::Connection *connection);

 private:

   void handleResponse( );

 private:
   basicSocket          _socket;
   AMQP::Connection*    _connection;
   AMQP::Channel *      _channel;
   bool                 _connected = false;
   bool                 _channelReady = false;
   std::string          _queueName;
   std::string          _exchangeName;
   CB                   _onMsgReceivedBC;

   std::string          _routingKey;
};
#endif
