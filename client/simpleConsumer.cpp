#include "simpleConsumer.h"
#include "clientImpl.h"
#include "simpleClient.h"
#include "RabbitMessage.h"

simpleConsumer::simpleConsumer(const ConnectionDetails& i_connectionDetails, 
        const std::string&  i_exchangeName, 
        ExchangeType        i_exchangeType,
        const std::string&  i_consumerID,
        CallbackType        i_onMessageCB,
        RabbitMQNotifiableIntf* i_handler,
        RabbitClientImpl* i_pOwner ):
    _connH( i_connectionDetails, [this] (const AMQP::Message & i_message) { return this->onMessageReceive ( &i_message ); } ),
    m_onMessageCB(i_onMessageCB),
    m_handler(i_handler),
    m_consumerID(i_consumerID),
    m_routingKey(i_consumerID),
    m_runStatus(RunStatus::Continue),
    m_exchangeName(i_exchangeName),
    m_exchageType(i_exchangeType),
    m_pOwner(i_pOwner)
{}
    
void simpleConsumer::operator()()
{
    RABBIT_DEBUG ("Consumer:: Consumer started ");
    while (1)
    {
        if ( _connH.login() == false )
        {
            RABBIT_DEBUG("Consumer:: Consumer failed to (re)connect. Exiting. ");
            return;
        }
        _connH.declareExchange(m_exchangeName.c_str() /*, ExchangeTypeStr[ (int)m_exchageType ]a*/ );
        _connH.declareQueue( m_consumerID.c_str() /*m_consumerID */); 
        BindMessage bindMessage(m_routingKey, m_consumerID, DeliveryType::Unicast);
        doBind(&bindMessage);

        rebind();
        while(1)
          _connH.receiveMessage( /*AMQP_NOACK */);
    }
} 

void simpleConsumer::stop(bool immediate)
{
    m_runStatus = (immediate) ? RunStatus::StopImmediate : RunStatus::StopGracefull;
    //m_rabbitProxy.stop();
}

int simpleConsumer::onMessageReceive(const AMQP::Message * i_message)
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
                    //assert (i_message->getQueue() == m_incomingMessages);
                    doBind(pBindMessage);
                }
                break;
            case MessageType::Unbind:
                {
                    UnbindMessage* pUnbindMessage = static_cast<UnbindMessage*>(pMessage);
                    //TODO: Important
                    //i_message->getQueue()->unBind( m_exchangeName, pUnbindMessage->unbindKey());
                    doUnbind( pUnbindMessage );
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
    _connH.bindQueueToExchange( i_pMessage->bindKey().c_str() );
}

void simpleConsumer::doUnbind(UnbindMessage* i_pMessage)
{
    _connH.unbindQueueToExchange( i_pMessage->unbindKey().c_str() );
}

RabbitMessageBase* simpleConsumer::AMQPMessageToRabbitMessage ( const AMQP::Message* i_message)
{
    std::string msg = i_message->message();
    std::string serializedMessage;
    serializedMessage.assign( msg );
    RabbitMessageBase* pMessage = RabbitMessageBase::deserialize(serializedMessage);
    if( pMessage == nullptr )
    {
      RABBIT_DEBUG("Consumer::Failed to deserialize message %s into RabbitMessage" << msg.c_str() )
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
