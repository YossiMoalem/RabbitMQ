#ifndef RABBIT_SOCKET
#define RABBIT_SOCKET

#include "ConnectionState.h"
#include "RabbitConnectionHandler.h" 
#include "RabbitSocket.h"

namespace AMQP {

class RabbitConnection
{
 public:

   RabbitConnection( OnMessageReveivedCB onMsgReceivedCB,
           ConnectionState & connectionState ) :
       _socket( std::make_shared< RabbitSocket > () ),
       _connectionState( connectionState ),
       _connectionHandler( std::make_shared< RabbitConnectionHandler > ( onMsgReceivedCB, connectionState, _socket) ),
       _onMsgReceivedCB( onMsgReceivedCB )
    { }

   RabbitConnection( OnMessageReveivedCB onMsgReceivedCB, const RabbitConnection & other ) :
       _socket( other._socket ),
       _connectionState( other._connectionState ),
       _connectionHandler( other._connectionHandler ),
       _onMsgReceivedCB( onMsgReceivedCB )
    { }

   void doPublish( const std::string & exchangeName,
           const std::string & routingKey, 
           const std::string & message, 
           DeferedResultSetter operationSucceeded ) const 
   { _connectionHandler->doPublish( exchangeName, routingKey, message, operationSucceeded ); }

   void doBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey,  
           DeferedResultSetter operationSucceeded ) const
   { _connectionHandler->doBindQueue( exchangeName, queueName, routingKey, operationSucceeded ); }

   void doUnBindQueue( const std::string & exchangeName, 
           const std::string & queueName, 
           const std::string & routingKey, 
           DeferedResultSetter operationSucceeded ) const
   { _connectionHandler->doUnBindQueue( exchangeName, queueName, routingKey, operationSucceeded ); }

   void declareExchange( const std::string & exchangeName, 
           ExchangeType type, 
           bool durable,
           DeferedResultSetter operationSucceeded ) const
   { _connectionHandler->declareExchange( exchangeName, type, durable, operationSucceeded ); }

   void declareQueue( const std::string & queueName, 
           bool durable, 
           bool exclusive, 
           bool autoDelete,
           DeferedResultSetter operationSucceeded ) const
   { _connectionHandler->declareQueue( queueName, durable, exclusive, autoDelete, operationSucceeded, _onMsgReceivedCB ); }

   void login( const std::string & userName, 
           const std::string & password,
           DeferedResultSetter operationSucceeded )
   { _connectionHandler->login( userName, password, operationSucceeded ); }

   void closeSocket()
   { _socket->close(); }

   bool pendingSend()
   { return _socket->pendingSend(); }

   unsigned int outgoingBufferSize() const
   { return _socket->outgoingBufferSize(); }

   //Do not cache. what if 2 Rabbit connection are using the same socket?
   int readFD() const 
   { return _readFD; }

   int writeFD() const 
   { return _writeFD; }

   void send( const char *data, size_t size)
   { _socket->send( data, size ); }

   void onMsgReceivedCB( const AMQP::Message message )
   { _onMsgReceivedCB( message ); }

   bool handleInput( )
   {
       const char* data;
       size_t size;
       if( _socket->read( data, size ) )
       {
           size_t processed = _connectionHandler->parse( data, size );
           _socket->shrink( processed );
           return true;
       }
       return false;
   }

   bool handleOutput()
   {
       return _socket->handleOutput();
   }

   bool connect(const RabbitConnectionDetails & connectionParams )
   {
       _connectionState.socketConnecting();
       _socket->clear();
       PRINT_DEBUG(DEBUG, "Connecting to : "<<connectionParams._host<<":"<<connectionParams._port);
       if( ! _socket->connect( connectionParams._host, connectionParams._port ) )
       {
           _connectionState.disconnected();
           PRINT_DEBUG(DEBUG, "Error creating socket");
           return false;
       } else {
           _readFD = _socket->readFD();
           _writeFD = _socket->writeFD();
           _connectionState.socketConnected();
           return true;
       }
   }

 private:
   std::shared_ptr< RabbitSocket >              _socket;
   int                                          _readFD;
   int                                          _writeFD;
   ConnectionState &                            _connectionState;
   std::shared_ptr< RabbitConnectionHandler >   _connectionHandler;
   std::function< int( const AMQP::Message& ) > _onMsgReceivedCB;
};
}
#endif
