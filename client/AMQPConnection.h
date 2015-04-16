#ifndef AMQP_CONNECTION_H
#define AMQP_CONNECTION_H

#include <boost/noncopyable.hpp>
#include <unordered_set>
#include "AMQPClient.h"
#include "Types.h"
#include "ConnectionDetails.h"

class AMQPConnection : boost::noncopyable
{
 public:
   AMQPConnection( const ConnectionDetails & connectionDetails,
           const std::string & exchangeName ,
           const std::string & queueName,
           const std::string & routingKey,
           AMQP::AMQPClient::OnMessageReveivedCB i_onMessageReceiveCB ) ;

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

 private:
   ReturnStatus _bind( const std::string & exchangeName,
           const std::string & queueName,
           const std::string routingKey) const;

   ReturnStatus _unBind( const std::string & exchangeName,
           const std::string & queueName,
           const std::string routingKey) const;

 private:
   AMQP::AMQPClient                 _connectionHandler;
   ConnectionDetails                _connectionDetails;
   bool                             _stop;
   std::atomic_bool                 _isConnected;
   std::thread                      _startLoopThread;
   std::string                      _exchangeName;
   std::string                      _queueName;
   std::string                      _routingKey;
   std::unordered_set< std::string> _bindingsSet;
   std::mutex                       _bindingsSetMutex;
};
#endif
