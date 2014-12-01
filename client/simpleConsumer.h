#ifndef SIMPLE_CONSUMER_H
#define SIMPLE_CONSUMER_H

#include <unordered_set>

#include "rabbitProxy.h"
#include "common.h" //for CAllbackType

class AMQPMessage;
class AMQPQueue;
class AMQPExchange;
class RabbitClientImpl;
class RabbitMQNotifiableIntf;

enum class ExchangeType;
enum class StopStatus;
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

   int bind(const std::string& i_key, DeliveryType i_deliveryType );
   int unbind(const std::string& i_key, DeliveryType i_deliveryType );

 private:
   int onMessageReceive(AMQPMessage* i_message);
   int rebind();
   int doBind(const std::string& i_key, DeliveryType i_deliveryType);

 private:
   CallbackType                   m_onMessageCB;
   RabbitMQNotifiableIntf*        m_handler;
   RabbitProxy                    m_rabbitProxy;
   const std::string              m_consumerID;
   AMQPQueue*                     m_incomingMessages;
   const std::string              m_routingKey;
   StopStatus                     m_stopStatus;
   AMQPExchange*                  m_exchange ;
   const std::string              m_exchangeName;
  ExchangeType                    m_exchageType;
   std::unordered_set<std::string> m_subscriptionsList;
   RabbitClientImpl*              m_pOwner;
};

#endif
