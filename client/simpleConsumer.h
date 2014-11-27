#ifndef SIMPLE_CONSUMER_H
#define SIMPLE_CONSUMER_H

#include <unordered_set>

#include "rabbitProxy.h"

class AMQPMessage;
class AMQPQueue;
class AMQPExchange;
class RabbitClientImpl;
class RabbitMQNotifiableIntf;
enum class ExchangeType;
enum class StopStatus;

class simpleConsumer : boost::noncopyable
{
 public:
   simpleConsumer( const connectionDetails& i_connectionDetails, 
       const std::string& i_exchangeName, 
       ExchangeType       i_exchangeType,
       const std::string& i_consumerID,
       int (*i_onMessageCB)(AMQPMessage*),
       RabbitMQNotifiableIntf* i_handler,
       RabbitClientImpl* i_pOwner );

   virtual void operator ()();
   virtual void stop(bool immediate);

   int bind(const std::string& i_key);
   int unbind(const std::string& i_key);

 public:
   static const char * const s_bindPrefix;
   static const char * const s_unbindPrefix;

 private:
   int onMessageReceive(AMQPMessage* i_message);
   int rebind();
   int doBind(const std::string& i_key);

 private:
   int (*m_onMessageCB)(AMQPMessage*);
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
