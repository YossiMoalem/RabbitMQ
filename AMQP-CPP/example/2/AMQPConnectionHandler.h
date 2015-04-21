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

class AMQPConnectionHandler : private AMQP::ConnectionHandler, boost::noncopyable 
{
 public:

   AMQPConnectionHandler( std::function<int( const AMQP::Message& )> onMsgReceivedCB );
   virtual ~AMQPConnectionHandler ();

   bool handleTimeout() const;

   void doPublish( const std::string & exchangeName,
           const std::string & routingKey, 
           const std::string & message, 
           RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const;

   void doBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey,  
           RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const;

   void doUnBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey, 
           RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const;

   bool handleInput( );
   bool handleOutput( );
   bool pendingSend();
   bool stopEventLoop();
   void setStopEventLoop( bool newBoolValue );

   void waitForConnection();
   /**
    * Blocking untill connection is either established or failes
    **/
   bool login( const AMQPConnectionDetails & connectionParams );

// protected:

   virtual void onConnected( AMQP::Connection *connection ) override ;

   virtual void onData(AMQP::Connection *connection, const char *data, size_t size) override;

   virtual void onError(AMQP::Connection *connection, const char *message) override;

   virtual void onClosed(AMQP::Connection *connection) override;

   std::future< bool > declareExchange( const std::string & exchangeName, 
           ExchangeType type = AMQP::fanout, 
           bool durable = false ) const ;

   std::future< bool > declareQueue( const std::string & queueName, 
           bool durable = false, 
           bool exclusive = false, 
           bool autoDelete = false ) const;

   int getReadFD() const;
   int getOutgoingMessagesFD() const;

    void closeSocket();
    void initTO() const ;
 private:
   AMQPSocket                       _socket;
   AMQP::Connection*                _connection;
   AMQP::Channel *                  _channel = nullptr;
   volatile bool                             _connected = false;
   volatile bool                             _stopEventLoop = false;
   std::function<int( const AMQP::Message& )> _onMsgReceivedBC;
   SmartBuffer                      _incomingMessages;
   SmartBuffer                      _outgoingMessages;
   std::mutex                       _connectionEstablishedMutex;
};

} //namespace AMQP
#endif
