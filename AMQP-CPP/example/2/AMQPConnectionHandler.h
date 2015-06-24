#ifndef AMQP_CLIENT_IMPLE_H
#define AMQP_CLIENT_IMPLE_H

#include <boost/noncopyable.hpp>
#include <memory>
#include <future>
#include <amqpcpp.h>
#include "AMQPSocket.h"

#include "ResultCodes.h"
#include "ConnectionState.h"

namespace AMQP {
class AMQPConnectionDetails;

class AMQPConnectionHandler : private AMQP::ConnectionHandler, boost::noncopyable 
{
    friend class Heartbeat;
 public:

   AMQPConnectionHandler( std::function<int( const AMQP::Message& )> onMsgReceivedCB,
           ConnectionState & connectionState ) ;

   virtual ~AMQPConnectionHandler ();

   //TODO: Need to prevent caling, or, trying to execute those function before we have a valid channed.
   void doPublish( const std::string & exchangeName,
           const std::string & routingKey, 
           const std::string & message, 
           DeferedResultSetter operationSucceeded ) const;

   void doBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey,  
           DeferedResultSetter operationSucceeded ) const;

   void doUnBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey, 
           DeferedResultSetter operationSucceeded ) const;

   void declareExchange( const std::string & exchangeName, 
           ExchangeType type, 
           bool durable,
           DeferedResultSetter operationSucceeded ) const;

   void declareQueue( const std::string & queueName, 
           bool durable, 
           bool exclusive, 
           bool autoDelete,
           DeferedResultSetter operationSucceeded ) const;

   void login( const std::string & userName, 
           const std::string & password,
           DeferedResultSetter operationSucceeded );

   bool handleInput( );
   bool handleOutput( );
   bool pendingSend();


// protected:


   virtual void onConnected( AMQP::Connection *connection ) override ;

   virtual void onData(AMQP::Connection *connection, const char *data, size_t size) override;

   virtual void onError(AMQP::Connection *connection, const char *message) override;

   virtual void onClosed(AMQP::Connection *connection) override;

   int getReadFD() const { return _socket.readFD(); }
   int getWriteFD() const { return _socket.writeFD(); }

    void closeSocket();
    bool connect(const AMQPConnectionDetails & connectionParams );
    unsigned int outgoingBufferSize() const;

 private:
   AMQPSocket                       _socket;
   AMQP::Connection*                _connection;
   AMQP::Channel *                  _channel = nullptr;
   std::function<int( const AMQP::Message& )> _onMsgReceivedBC;
   SmartBuffer                      _incomingMessages;
   SmartBuffer                      _outgoingBuffer;
   ConnectionState &                _connectionState;
};

} //namespace AMQP
#endif
