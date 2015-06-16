#ifndef AMQP_CLIENT_IMPLE_H
#define AMQP_CLIENT_IMPLE_H

#include <boost/noncopyable.hpp>
#include <memory>
#include <future>
#include <amqpcpp.h>
#include "AMQPSocket.h"
#include "RabbitOperation.h"


namespace AMQP {
class AMQPConnectionDetails;
class Heartbeat;

class AMQPConnectionHandler : private AMQP::ConnectionHandler, boost::noncopyable 
{
    friend class Heartbeat;
 public:

   AMQPConnectionHandler( std::function<int( const AMQP::Message& )> onMsgReceivedCB, AMQPEventLoop * eventloop );
   virtual ~AMQPConnectionHandler ();

   bool handleTimeout() const;

   void doPublish( const std::string & exchangeName,
           const std::string & routingKey, 
           const std::string & message, 
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

   void doBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey,  
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

   void doUnBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey, 
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

   void declareExchange( const std::string & exchangeName, 
           ExchangeType type, 
           bool durable,
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

   void declareQueue( const std::string & queueName, 
           bool durable, 
           bool exclusive, 
           bool autoDelete,
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const;

   void login( const std::string & userName, 
           const std::string & password,
           RabbitMessageBase::DeferedResultSetter operationSucceeded );

   bool handleInput( );
   bool handleOutput( );
   bool pendingSend();


// protected:

   virtual void onConnected( AMQP::Connection *connection ) override ;

   virtual void onData(AMQP::Connection *connection, const char *data, size_t size) override;

   virtual void onError(AMQP::Connection *connection, const char *message) override;

   virtual void onClosed(AMQP::Connection *connection) override;

   int getReadFD() const;
   int getOutgoingMessagesFD() const;

    void closeSocket();
    bool openConnection(const AMQPConnectionDetails & connectionParams );

 private:
   AMQPSocket                       _socket;
   AMQP::Connection*                _connection;
   AMQP::Channel *                  _channel = nullptr;
   volatile bool                             _connected = false;
   std::function<int( const AMQP::Message& )> _onMsgReceivedBC;
   SmartBuffer                      _incomingMessages;
   SmartBuffer                      _outgoingMessages;
   std::unique_ptr< Heartbeat >     _heartbeat;
   RabbitMessageBase::DeferedResultSetter _loginValueSetter;
   //TODO: this is a temporary WA. 
   //Till login will be moved to the eventloop thread, we need to release it
   //if event loop is terminated.
   //for this we want oto ask the event lop if it is still running
   AMQPEventLoop *                  _eventLoop;
};

} //namespace AMQP
#endif
