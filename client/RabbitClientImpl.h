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
   RabbitClientImpl(const ConnectionDetails & _connectionDetails, 
           const std::string& _exchangeName,
           const std::string& _lucExchangeName,
           const std::string& _consumerID,
           CallbackType       _onMessageCB );

   ReturnStatus start();

   ReturnStatus stop( bool immediate );

   ReturnStatus sendMessage(const std::string& _message,
       const std::string& _destination,
       const std::string& _senderID,
       DeliveryType _deliveryType) const;

   ReturnStatus sendMessage(const std::string& _message, 
       const std::string& _destination, 
       const std::string& _senderID,
       const std::string& _excName,
       DeliveryType _deliveryType) const;

   ReturnStatus bind(const std::string& _key, DeliveryType _deliveryType);

   ReturnStatus unbind(const std::string& _key, DeliveryType _deliveryType);

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
   const std::string        _lucExchangeName;
   const std::string        _queueName;
   //CallbackType             _onMessageReceivedCB;
   mutable CallbackHandler          _callbackHandler;
};


#endif
