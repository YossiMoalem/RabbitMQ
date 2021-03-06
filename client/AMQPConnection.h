#ifndef AMQP_CONNECTION_H
#define AMQP_CONNECTION_H

#include <boost/noncopyable.hpp>
#include <unordered_set>
#include <RabbitClient.h>
#include "Types.h"
#include "ConnectionDetails.h"

class AMQPConnection : boost::noncopyable
{
 public:
   AMQPConnection( const ConnectionDetails & connectionDetails,
           const std::vector< std::string > & exchangesName ,
           const std::string & queueName,
           const std::string & routingKey,
           AMQP::OnMessageReceivedCB i_onMessageReceiveCB ) ;

   ReturnStatus start();

   ReturnStatus connectLoop();

   ReturnStatus stop( bool immediate );

   ReturnStatus publish( const std::string & exchangeName, 
           const std::string & routingKey,
           const std::string & message ) const;

   ReturnStatus bind( const std::string & exchangeName,
           const std::string & queueName,
           const std::string routingKey);

   ReturnStatus unBind( const std::string & exchangeName,
           const std::string & queueName,
           const std::string routingKey);

   bool connected() const;
   ReturnStatus rebind();
   bool declareExchange ( const std::string & exchangeName, unsigned int waitTime ) const;

 private:
   ReturnStatus _bind( const std::string & exchangeName,
           const std::string & queueName,
           const std::string routingKey) const;

   ReturnStatus _unBind( const std::string & exchangeName,
           const std::string & queueName,
           const std::string routingKey) const;

   bool _declareExchanges() const;
   bool _login() const;
   bool _declareQueue() const;
   bool _removeQueue() const;

 private:
   AMQP::RabbitClient                 _connectionHandler;
   ConnectionDetails                _connectionDetails;
   bool                             _stop;
   std::atomic_bool                 _isConnected;
   std::thread                      _startLoopThread;
   const std::vector< std::string > & _exchangesName;
   std::string                      _queueName;
   std::string                      _routingKey;
   std::unordered_set< std::string> _bindingsSet;
   std::mutex                       _bindingsSetMutex;
};
#endif
