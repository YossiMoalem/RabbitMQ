#ifndef CLIENT_IMPL_H
#define CLIENT_IMPL_H

#include <boost/noncopyable.hpp>
#include <string>

#include "AMQPConnection.h"
#include "CallbackHandler.h"

class ConnectionDetails;
namespace AMQP{
    class Message; 
}

class RabbitClientImpl : public boost::noncopyable
{
 public:
   RabbitClientImpl(const ConnectionDetails & connectionDetails, 
           const std::string& exchangeName,
           const std::string& consumerID,
           CallbackType       onMessageCallback );

   ReturnStatus start();

   ReturnStatus stop( bool immediate );

   ReturnStatus sendMessage(const std::string& message,
       const std::string& destination,
       const std::string& senderID,
       DeliveryType deliveryType) const;

   ReturnStatus sendMessage(const std::string& message, 
       const std::string& destination, 
       const std::string& senderID,
       const std::string& exchangeName,
       DeliveryType deliveryType) const;

   ReturnStatus bind(const std::string& key, DeliveryType deliveryType);

   ReturnStatus unbind(const std::string& key, DeliveryType deliveryType);

   bool         connected () const;

   int onMessageReceived( const AMQP::Message & message ) const;

   static std::string serializePostMessage( const std::string & sender,
           const std::string & destination,
           DeliveryType deliveryType,
           const std::string & message);

   static void deserializePostMessage( const std::string serializedMessage, 
           std::string & sender,
           std::string & destination,
           DeliveryType & deliveryType,
           std::string & message);

   private:
   std::string createRoutingKey( const std::string & sender, 
                    const std::string & destination,
                    DeliveryType deliveryType ) const;

 private:
   AMQPConnection           _AMQPConnection;
   const std::string        _exchangeName;
   const std::string        _queueName;
   mutable CallbackHandler          _callbackHandler;
};


#endif
