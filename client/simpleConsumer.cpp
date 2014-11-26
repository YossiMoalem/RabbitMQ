#include "simpleConsumer.h"
#include "clientImpl.h"
#include "common.h"
#include "simpleClient.h"

#include <AMQPcpp.h>

const char * const simpleConsumer::s_bindPrefix = "BIND::";
const char * const simpleConsumer::s_unbindPrefix = "UNBIND::";

simpleConsumer::simpleConsumer(const connectionDetails& i_connectionDetails, 
        const std::string& i_exchangeName, 
        const std::string& i_consumerID,
        int (*i_onMessageCB)(AMQPMessage*),
        RabbitMQNotifiableIntf* i_handler,
        RabbitClientImpl* i_pOwner ):
    m_onMessageCB(i_onMessageCB),
    m_handler(i_handler),
    m_rabbitProxy(i_connectionDetails),
    m_consumerID(i_consumerID),
    m_routingKey(i_consumerID),
    m_stopStatus(SS_Continue),
    m_exchange(NULL),
    m_exchangeName(i_exchangeName),
    m_pOwner(i_pOwner)
{}
    
void simpleConsumer::operator()()
{
    RABBIT_DEBUG ("Consumer:: Consumer started ");
    while (1)
    {
      m_rabbitProxy.init();
    m_exchange = m_rabbitProxy.m_connectionHolder->createExchange(m_exchangeName);
    //TODO: yet, another crap allert. 
    //This is just to make Adam (adam@liveu.tv) happy.
    //Once I change the type to be enum, 
    //use it here. Cannot be bothrerd with crap of stings that will be deleted soon anyway
    m_exchange->Declare(m_exchangeName, "direct");

      m_incomingMessages = m_rabbitProxy.m_connectionHolder->createQueue(m_consumerID); 
      m_incomingMessages->Declare(m_consumerID); 
      //TODO: really???
      m_incomingMessages->Bind( m_exchangeName, std::string("ALL:") + m_routingKey);

      m_incomingMessages->addEvent(AMQP_MESSAGE, [this] (AMQPMessage* i_message) { return this->onMessageReceive (i_message); } );
      //          m_incomingMessages->addEvent(AMQP_CANCEL, m_handler->onCancel );
      rebind();
      m_incomingMessages->Consume(AMQP_NOACK);
    }
} 

void simpleConsumer::stop(bool immediate)
{
    m_stopStatus = (immediate) ? SS_StopImmediate : SS_StopGracefull;
    //TODO: and??
}

int simpleConsumer::onMessageReceive(AMQPMessage* i_message)
{
    uint32_t messageLength = 0;
    const char * msg = i_message->getMessage(&messageLength);
    std::string message_text;
    message_text.assign(msg, messageLength);

    RABBIT_DEBUG ("Consumer:: got message : " <<message_text);
  //TODO: I asume the message length is greater than BIND_PREFIX and UNBIND_PREFIX
  if (message_text.compare (0, strlen( s_bindPrefix ), s_bindPrefix ) == 0 ) 
  {
    RABBIT_DEBUG ("Consumer:: Going to bind to "<< message_text.substr(strlen( s_bindPrefix ) ));
    i_message->getQueue()->Bind( m_exchangeName, message_text.substr(strlen( s_bindPrefix ) ) );
    return 0;
  }

  if ( message_text.compare (0, strlen( s_unbindPrefix ), s_unbindPrefix ) == 0 ) 
  {
    RABBIT_DEBUG ("Consumer:: Going to unbind from  "<< message_text.substr(strlen( s_unbindPrefix ) ));
    i_message->getQueue()->unBind( m_exchangeName, message_text.substr(strlen( s_unbindPrefix ) ) );
    return 0;
  }
  int status = 0;
  if ( m_onMessageCB )
      status = (*m_onMessageCB)(i_message);
  if (m_handler)
    status = m_handler->onMessageReceive(i_message);
  return status;
}

int simpleConsumer::rebind()
{
    for( auto key : m_subscriptionsList )
    {
        doBind(key);
    }
    return 0;
}
int simpleConsumer::bind(const std::string& i_key)
{ 
  m_subscriptionsList.insert(i_key);
  return doBind(i_key);
}

int simpleConsumer::unbind(const std::string& i_key)
{ 
  std::string unbindMessage ( simpleConsumer::s_unbindPrefix + i_key );
  m_subscriptionsList.erase(i_key);
  return m_pOwner->sendUnicast( unbindMessage, m_consumerID);
}


int simpleConsumer::doBind(const std::string& i_key)
{
  std::string bindMessage ( simpleConsumer::s_bindPrefix + i_key );
  return m_pOwner->sendUnicast( bindMessage, m_consumerID);
}
