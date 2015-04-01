#include "RabbitClient.h"
#include "clientImpl.h"

/*
RabbitClient::RabbitClient(const ConnectionDetails & i_connectionDetails, 
    const std::string& i_exchangeName, 
    const std::string& i_consumerID,
    RabbitMQNotifiableIntf* i_handler) :
  m_pRabbitClient( new RabbitClientImpl (i_connectionDetails,
        i_exchangeName,
        i_consumerID,
        i_handler))
{}
*/

RabbitClient::RabbitClient(const ConnectionDetails & i_connectionDetails, 
    const std::string& i_exchangeName, 
    const std::string& i_consumerID,
    CallbackType        i_onMessageCB ) :
  m_pRabbitClient( new RabbitClientImpl (i_connectionDetails,
        i_exchangeName,
        i_consumerID,
        i_onMessageCB ) )
{}

ReturnStatus RabbitClient::start()
{ 
    return m_pRabbitClient->start(); 
}

ReturnStatus RabbitClient::stop(bool immediate)
{ 
    return m_pRabbitClient->stop(immediate);
}

ReturnStatus RabbitClient::sendUnicast(const std::string& i_message, 
        const std::string& i_destination, 
        const std::string& i_senderID)
{ 
    return m_pRabbitClient->sendMessage(i_message,i_destination, i_senderID, DeliveryType::Unicast); 
}

ReturnStatus RabbitClient::sendMulticast(const std::string& i_message, const std::string& i_senderID)
{ 
    return m_pRabbitClient->sendMessage(i_message, "", i_senderID, DeliveryType::Multicast); 
}

ReturnStatus RabbitClient::bindToSelf(const std::string& i_key)
{ 
    return m_pRabbitClient->bind(i_key, DeliveryType::Unicast); 
}

ReturnStatus RabbitClient::bindToDestination(const std::string& i_key)
{ 
    return m_pRabbitClient->bind(i_key, DeliveryType::Multicast); 
}

ReturnStatus RabbitClient::unbindFromSelf(const std::string& i_key)
{ 
    return m_pRabbitClient->unbind(i_key, DeliveryType::Unicast); 
}

ReturnStatus RabbitClient::unbindFromDestination(const std::string& i_key)
{ 
    return m_pRabbitClient->unbind(i_key, DeliveryType::Multicast); 
}

bool RabbitClient::isConnected() const
{
  return m_pRabbitClient->connected();
}
