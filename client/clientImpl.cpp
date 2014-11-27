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
        int (*i_onMessageCB)(AMQPMessage*) ) :
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

int RabbitClientImpl::sendMessage(const std::string& i_message, 
        const std::string& i_destination, 
        const std::string& i_senderID, 
        DeliveryType i_deliveryType)
{
    return sendRawMessage(new PostMessage(i_message, i_destination, i_senderID, i_deliveryType ) );
}

int RabbitClientImpl::sendRawMessage(RabbitMessageBase* i_pMessage)
{
    //TODO: check  if publisher is connected. If not - only send bind commands
    //TODO: write more info 
    RABBIT_DEBUG ("Client:: Going to push msg type "<< (int) i_pMessage->messageType()
            << " to : " << i_pMessage->getDestination() );
    m_messageQueueToSend.push( i_pMessage );
    return 0;
}

int RabbitClientImpl::bind(const std::string& i_key, DeliveryType i_deliveryType)
{ 
  return m_consumer.bind( i_key, i_deliveryType );
}

int RabbitClientImpl::unbind(const std::string& i_key, DeliveryType i_deliveryType)
{ 
  return m_consumer.unbind( i_key, i_deliveryType );
}
