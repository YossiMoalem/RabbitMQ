#include "simpleConsumer.h"
#include "clientImpl.h"
#include "simpleClient.h"

#include <AMQPcpp.h>

simpleConsumer::simpleConsumer(const connectionDetails& i_connectionDetails, 
        const std::string&  i_exchangeName, 
        ExchangeType        i_exchangeType,
        const std::string&  i_consumerID,
        CallbackType        i_onMessageCB,
        RabbitMQNotifiableIntf* i_handler,
        RabbitClientImpl* i_pOwner ):
    m_onMessageCB(i_onMessageCB),
    m_handler(i_handler),
    m_rabbitProxy(i_connectionDetails),
    m_consumerID(i_consumerID),
    m_routingKey(i_consumerID),
    m_runStatus(RunStatus::Continue),
    m_exchange(NULL),
    m_exchangeName(i_exchangeName),
    m_exchageType(i_exchangeType),
    m_pOwner(i_pOwner)
{}
    
void simpleConsumer::operator()()
{
    RABBIT_DEBUG ("Consumer:: Consumer started ");
    while (1)
    {
        if (m_rabbitProxy.init() == false )
        {
            RABBIT_DEBUG("Consumer:: Consumer failed to (re)connect. Exiting. ");
            return;
        }
        m_exchange = m_rabbitProxy.m_connectionHolder->createExchange(m_exchangeName);
        m_exchange->Declare(m_exchangeName, ExchangeTypeStr[ (int)m_exchageType ] );

        m_incomingMessages = m_rabbitProxy.m_connectionHolder->createQueue(m_consumerID); 
        m_incomingMessages->Declare(m_consumerID); 
        BindMessage bindMessage(m_routingKey, m_consumerID, DeliveryType::Unicast);
        doBind(&bindMessage);

        m_incomingMessages->addEvent(AMQP_MESSAGE, [this] (AMQPMessage* i_message) { return this->onMessageReceive (i_message); } );
        //          m_incomingMessages->addEvent(AMQP_CANCEL, m_handler->onCancel );
        rebind();
        m_incomingMessages->Consume(AMQP_NOACK);
    }
} 

void simpleConsumer::stop(bool immediate)
{
    m_runStatus = (immediate) ? RunStatus::StopImmediate : RunStatus::StopGracefull;
    m_rabbitProxy.stop();
}

int simpleConsumer::onMessageReceive(AMQPMessage* i_message)
{
    if (m_runStatus == RunStatus::StopImmediate)
    {
        RABBIT_DEBUG("Consumer:: Consumer got StopImmediate command.Exiting");
        pthread_exit(nullptr);
    }
    RabbitMessageBase* pMessage = AMQPMessageToRabbitMessage(i_message);
    int status = 0;
    if (pMessage != nullptr)
    {
        RABBIT_DEBUG("Consumer:: Got message: " <<*pMessage );
        switch (pMessage->messageType())
        {
            case MessageType::Post:
                {
                    PostMessage* pPostMessage = static_cast<PostMessage*>(pMessage);
                    if ( m_onMessageCB )
                        status = (m_onMessageCB)(pPostMessage->m_sender, 
                                    pPostMessage->m_destination, 
                                    pPostMessage->deliveryType(), 
                                    pPostMessage->getText());
                    if (m_handler)
                        status = m_handler->onMessageReceive(pPostMessage->m_sender, 
                                        pPostMessage->m_destination, 
                                        pPostMessage->deliveryType(), 
                                        pPostMessage->getText());
                }
                break;
            case MessageType::Bind:
                {
                    BindMessage* pBindMessage = static_cast<BindMessage*>(pMessage);
                    assert (i_message->getQueue() == m_incomingMessages);
                    doBind(pBindMessage);
                }
                break;
            case MessageType::Unbind:
                {
                    UnbindMessage* pUnbindMessage = static_cast<UnbindMessage*>(pMessage);
                    i_message->getQueue()->unBind( m_exchangeName, pUnbindMessage->unbindKey());
                }
                break;
            default:
                {
                    RABBIT_DEBUG("Consumer:: Unknown Message type : " << (int)pMessage->messageType());
                }
        }
        delete pMessage;
    }
    if (m_runStatus != RunStatus::Continue)
    {
        RABBIT_DEBUG("Consumer:: Consumer got Stop command.Exiting");
        pthread_exit(nullptr);
    }
    return status;
}

void simpleConsumer::doBind(BindMessage* i_pMessage)
{
    m_incomingMessages->Bind( m_exchangeName, i_pMessage->bindKey(), false );
}

RabbitMessageBase* simpleConsumer::AMQPMessageToRabbitMessage (AMQPMessage* i_message)
{
    uint32_t messageLength = 0;
    const char * msg = i_message->getMessage(&messageLength);
    std::string serializedMessage;
    serializedMessage.assign(msg, messageLength);
    RabbitMessageBase* pMessage = RabbitMessageBase::deserialize(serializedMessage);
    if( pMessage == nullptr )
    {
      RABBIT_DEBUG("Consumer::Failed to deserialize message %s into RabbitMessage" << *msg )
    }
    return pMessage;
}

int simpleConsumer::rebind()
{
    for( auto key : m_subscriptionsList )
    {
        sendBindMessage(key.first, (DeliveryType)key.second);
    }
    return 0;
}
ReturnStatus simpleConsumer::bind(const std::string& i_key, DeliveryType i_deliveryType)
{ 
  m_subscriptionsList.insert(std::pair<std::string, int>(i_key, (int)i_deliveryType) );
  return sendBindMessage(i_key, i_deliveryType);
}

ReturnStatus simpleConsumer::unbind(const std::string& i_key, DeliveryType i_deliveryType)
{ 
  m_subscriptionsList.erase(std::pair<std::string, int> (i_key, (int)i_deliveryType) );
  return m_pOwner->sendMessage( new UnbindMessage (i_key, m_consumerID, i_deliveryType));
}


ReturnStatus simpleConsumer::sendBindMessage(const std::string& i_key, DeliveryType i_deliveryType)
{
  return m_pOwner->sendMessage(new BindMessage(i_key, m_consumerID, i_deliveryType));
}
