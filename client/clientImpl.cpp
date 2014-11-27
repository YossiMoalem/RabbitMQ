#include "clientImpl.h"
#include <boost/ref.hpp>

#include <AMQPcpp.h>

#define SELF_ROUTING(KEY) "ALL:"+KEY
#define DESTINATION_ROUTING(KEY)  KEY+":ALL"

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
    //TODO: Think: should creating the threads be the client responsibility, or
    //should the workers export "start" method that will spawn the threadsm, and 
    //client should only call it??
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

int RabbitClientImpl::sendUnicast(const std::string& i_message, const std::string& i_destination) 
{
    return send(i_message, SELF_ROUTING( i_destination ) );
}

int RabbitClientImpl::sendMulticast(const std::string& i_message, const std::string& i_destination)
{
    return send(i_message, DESTINATION_ROUTING( i_destination ) );
}

int RabbitClientImpl::send(const std::string& i_message, 
                    const std::string& i_destination)
{
    //TODO: check  if publisher is connected. If not - only send bind commands
    RABBIT_DEBUG ("Client:: Going to push msg: "<< i_message << " to : " << i_destination );
    m_messageQueueToSend.push(Protocol(i_message, i_destination ));
    return 0;
}

int RabbitClientImpl::bindToSelf(const std::string& i_key)
{ 
  return m_consumer.bind( SELF_ROUTING( i_key ) );
}

int RabbitClientImpl::bindToDestination(const std::string& i_key)
{ 
  return m_consumer.bind( DESTINATION_ROUTING( i_key ) );
}

int RabbitClientImpl::unbindFromSelf(const std::string& i_key)
{ 
  return m_consumer.unbind( SELF_ROUTING( i_key ) );
}

int RabbitClientImpl::unbindFromDestination(const std::string& i_key)
{ 
  return m_consumer.unbind( DESTINATION_ROUTING( i_key ) );
}
