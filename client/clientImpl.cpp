#include "clientImpl.h"
#include <boost/ref.hpp>

#include <AMQPcpp.h>

RabbitClientImpl::RabbitClientImpl(const connectionDetails& i_connectionDetails, 
        const std::string& i_exchangeName, 
        const std::string& i_consumerID,
        ExchangeType       i_exchangeType,
        RabbitMQNotifiableIntf* i_handler) :
    m_connectionDetails(i_connectionDetails),
    m_exchangeName(i_exchangeName),
    m_consumerID(i_consumerID),
    m_onMessageCB(nullptr),
    m_handler(i_handler),
    m_publisher(m_connectionDetails, m_exchangeName, i_exchangeType, m_consumerID, m_messageQueueToSend),
    m_consumer(m_connectionDetails, m_exchangeName, i_exchangeType, m_consumerID, m_onMessageCB, m_handler, this)
{}

RabbitClientImpl::RabbitClientImpl(const connectionDetails& i_connectionDetails, 
        const std::string& i_exchangeName, 
        const std::string& i_consumerID,
        ExchangeType       i_exchangeType,
        CallbackType        i_onMessageCB ) :
    m_connectionDetails(i_connectionDetails),
    m_exchangeName(i_exchangeName),
    m_consumerID(i_consumerID),
    m_onMessageCB(i_onMessageCB),
    m_handler(nullptr),
    m_publisher(m_connectionDetails, m_exchangeName, i_exchangeType, m_consumerID, m_messageQueueToSend),
    m_consumer(m_connectionDetails, m_exchangeName, i_exchangeType, m_consumerID, m_onMessageCB, m_handler, this)
{}

int RabbitClientImpl::start()
{
    m_threads.create_thread( boost::ref(m_publisher) );
    m_threads.create_thread( boost::ref(m_consumer) );
    return 0;
}

int RabbitClientImpl::stop(bool immediate)
{
    m_publisher.stop(immediate);
    m_consumer.stop(immediate);
    m_threads.join_all();
    return 0;
}

ReturnStatus RabbitClientImpl::sendMessage(const std::string& i_message, 
        const std::string& i_destination, 
        const std::string& i_senderID, 
        DeliveryType i_deliveryType)
{
    PostMessage* newMessage = new PostMessage(i_message, i_destination, i_senderID, i_deliveryType );
    return sendMessage(newMessage);
}
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
    if (status == MessageQueue::ReturnStatus::QueueOpenForHigPriorityDataOnly)
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

ReturnStatus RabbitClientImpl::bind(const std::string& i_key, DeliveryType i_deliveryType)
{ 
  return m_consumer.bind( i_key, i_deliveryType );
}

ReturnStatus RabbitClientImpl::unbind(const std::string& i_key, DeliveryType i_deliveryType)
{ 
  return m_consumer.unbind( i_key, i_deliveryType );
}
