#ifndef CLIENT_IMPL_H
#define CLIENT_IMPL_H

#include <boost/noncopyable.hpp>
#include <string>

#include "AMQPConnection.h"

class ConnectionDetails;
namespace AMQP{
    class Message; 
}

class RabbitClientImpl : public boost::noncopyable
{
 public:
   RabbitClientImpl(const ConnectionDetails & i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           CallbackType       i_onMessageCB );

   ReturnStatus start();

   ReturnStatus stop( bool immediate );

   ReturnStatus sendMessage(const std::string& i_message, 
       const std::string& i_destination, 
       const std::string& i_senderID, 
       DeliveryType i_deliveryType) const;

   ReturnStatus bind(const std::string& i_key, DeliveryType i_deliveryType);

   ReturnStatus unbind(const std::string& i_key, DeliveryType i_deliveryType);

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
   CallbackType             _onMessageReceivedCB;
};


#endif
