#ifndef SIMPLE_CONSUMER_H
#define SIMPLE_CONSUMER_H

#include <unordered_set>
#include <boost/functional/hash.hpp>
#include <boost/noncopyable.hpp>

#include <myConnectionHandler.h>

#include "internalTypes.h"
#include "ConnectionDetails.h"
namespace AMQP {
class Message;
}
class RabbitClientImpl;
class RabbitMQNotifiableIntf;
class BindMessage;
class UnbindMessage;

enum class ExchangeType;
enum class RunStatus;
enum class DeliveryType;

class simpleConsumer : boost::noncopyable
{
 public:
   simpleConsumer( const ConnectionDetails& i_connectionDetails, 
       const std::string& i_exchangeName, 
       ExchangeType       i_exchangeType,
       const std::string& i_consumerID,
       CallbackType       i_onMessageCB,
       RabbitMQNotifiableIntf* i_handler,
       RabbitClientImpl* i_pOwner );

   virtual void operator ()();
   virtual void stop(bool immediate);

   ReturnStatus bind(const std::string& i_key, DeliveryType i_deliveryType );
   ReturnStatus unbind(const std::string& i_key, DeliveryType i_deliveryType );

 private:
   int onMessageReceive( const AMQP::Message * i_message);
   int rebind();
   ReturnStatus sendBindMessage(const std::string& i_key, DeliveryType i_deliveryType);
   RabbitMessageBase* AMQPMessageToRabbitMessage ( const AMQP::Message *  i_message);
   void doBind (BindMessage* i_message);
   void doUnbind (UnbindMessage* i_message);

 private:
   ConnectionDetails            _connectionDetails;
   MyConnectionHandler          _connH;
   CallbackType                   m_onMessageCB;
   RabbitMQNotifiableIntf*        m_handler;
   const std::string              m_queueName;
   const std::string              m_routingKey;
   RunStatus                  m_runStatus;
   const std::string              m_exchangeName;
   ExchangeType                    m_exchageType;
   RabbitClientImpl*              m_pOwner;
   std::unordered_set<
       std::pair <std::string, int>, 
       boost::hash <std::pair< std::string, int> >  
           > m_subscriptionsList;
};

#endif
