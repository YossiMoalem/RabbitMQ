#ifndef SIMPLE_CONSUMER_H
#define SIMPLE_CONSUMER_H

#include <unordered_set>
#include <boost/functional/hash.hpp>

#include "rabbitProxy.h"
#include "internalTypes.h"

class AMQPMessage;
class AMQPQueue;
class AMQPExchange;
class RabbitClientImpl;
class RabbitMQNotifiableIntf;
class BindMessage;

enum class ExchangeType;
enum class RunStatus;
enum class DeliveryType;

class simpleConsumer : boost::noncopyable
{
 public:
   simpleConsumer( const connectionDetails& i_connectionDetails, 
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
   int onMessageReceive(AMQPMessage* i_message);
   int rebind();
   ReturnStatus sendBindMessage(const std::string& i_key, DeliveryType i_deliveryType);
   RabbitMessageBase* AMQPMessageToRabbitMessage (AMQPMessage* i_message);
   void doBind (BindMessage* i_message);

 private:
   CallbackType                   m_onMessageCB;
   RabbitMQNotifiableIntf*        m_handler;
   RabbitProxy                    m_rabbitProxy;
   const std::string              m_consumerID;
   AMQPQueue*                     m_incomingMessages;
   const std::string              m_routingKey;
   RunStatus                  m_runStatus;
   AMQPExchange*                  m_exchange ;
   const std::string              m_exchangeName;
  ExchangeType                    m_exchageType;
   RabbitClientImpl*              m_pOwner;
   std::unordered_set<
       std::pair <std::string, int>, 
       boost::hash <std::pair< std::string, int> >  
           > m_subscriptionsList;
};

#endif
