#include "clientImpl.h"
#include <boost/ref.hpp>
#include <signal.h>//for pthread kill

#define UNICAST_PREFIX "ALL:"
#define MULTICAST_SUFFIX ":ALL"
/*
RabbitClientImpl::RabbitClientImpl(const ConnectionDetails & i_connectionDetails, 
        const std::string& i_exchangeName, 
        const std::string& i_consumerID,
        RabbitMQNotifiableIntf* i_handler) :
    _AMQPConnection(i_connectionDetails),
    m_exchangeName(i_exchangeName),
    m_consumerID(i_consumerID),
    m_onMessageCB(nullptr),
    m_handler(i_handler),
    m_publisher(m_connectionDetails, m_exchangeName, m_consumerID, m_messageQueueToSend),
    m_consumer(m_connectionDetails, m_exchangeName, m_consumerID, m_onMessageCB, m_handler, this)
{}
*/

RabbitClientImpl::RabbitClientImpl(const ConnectionDetails & i_connectionDetails, 
        const std::string& i_exchangeName, 
        const std::string& i_consumerID,
        CallbackType        i_onMessageCB ) :
    _AMQPConnection(i_connectionDetails, i_exchangeName, i_consumerID, i_onMessageCB ),
    _exchangeName(i_exchangeName),
    _queueName(i_consumerID)/*,
    m_onMessageCB(i_onMessageCB),
    m_handler(nullptr),
    m_publisher(m_connectionDetails, m_exchangeName, m_consumerID, m_messageQueueToSend),
    m_consumer(m_connectionDetails, m_exchangeName, m_consumerID, m_onMessageCB, m_handler, this)
    */
{}

ReturnStatus RabbitClientImpl::start()
{
    return _AMQPConnection.start();
}

ReturnStatus RabbitClientImpl::stop( bool immediate )
{
    return _AMQPConnection.stop( immediate );
}

ReturnStatus RabbitClientImpl::sendMessage(const std::string& i_message, 
        const std::string& i_destination, 
        const std::string& i_senderID, 
        DeliveryType i_deliveryType)
{
    std::string routingKey;
    if( i_deliveryType == DeliveryType::Unicast )
        routingKey = UNICAST_PREFIX + i_destination;
    else
        routingKey = i_senderID + MULTICAST_SUFFIX;
    _AMQPConnection.publish( _exchangeName, routingKey, i_message );
    //PostMessage* newMessage = new PostMessage(i_message, i_destination, i_senderID, i_deliveryType );
    //return sendMessage(newMessage);
    return ReturnStatus::Ok;
}
/*
ReturnStatus RabbitClientImpl::sendMessage(BindMessage*   i_bindMessage)
{
    return doSendMessage(i_bindMessage, true);
}
ReturnStatus RabbitClientImpl::sendMessage(UnbindMessage* i_unbindMessage)
{
    return doSendMessage(i_unbindMessage, true);
}
ReturnStatus RabbitClientImpl::sendMessage(PostMessage*   i_postMessage)
{
    return doSendMessage(i_postMessage, false);
}

ReturnStatus RabbitClientImpl::doSendMessage(RabbitMessageBase* i_message, bool highPriority)
{
    RABBIT_DEBUG("Client:: Going to push message: " << *i_message);
    MessageQueue::ReturnStatus status = m_messageQueueToSend.push(i_message, highPriority);
    if (status == MessageQueue::ReturnStatus::QueueOpenForAdminMessagesOnly)
    {
        RABBIT_DEBUG("Client:: Message Dropped because publisher is disconnected");
        return ReturnStatus::ClientDisconnected;
    }
    if(status == MessageQueue::ReturnStatus::QueueBlocked)
    {
        RABBIT_DEBUG("Client:: Message Dropped because client shutting down");
        return ReturnStatus::ClientSuttingDown;
    }
    assert (status == MessageQueue::ReturnStatus::Ok);
    return ReturnStatus::Ok;
}
*/

ReturnStatus RabbitClientImpl::bind(const std::string& i_key, DeliveryType i_deliveryType)
{ 
 //Add to subscription list!!
    std::string routingKey;
    if (i_deliveryType == DeliveryType::Unicast)
        routingKey = UNICAST_PREFIX + i_key;
    else
        routingKey = i_key+ MULTICAST_SUFFIX;

  return _AMQPConnection.bind( _exchangeName, _queueName, routingKey );
}

ReturnStatus RabbitClientImpl::unbind(const std::string& i_key, DeliveryType i_deliveryType)
{ 
    //Remove from subscroption list!
    std::string routingKey;
    if (i_deliveryType == DeliveryType::Unicast)
        routingKey = UNICAST_PREFIX + i_key;
    else
        routingKey = i_key+ MULTICAST_SUFFIX;

  return _AMQPConnection.unBind( _exchangeName, _queueName, routingKey );
}

bool RabbitClientImpl::connected() const
{
  return _AMQPConnection.connected();
}
