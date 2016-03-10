#include "RabbitClient.h"
#include "RabbitClientImpl.h"

RabbitClient::RabbitClient( const ConnectionDetails & connectionDetails, 
    const std::string & defaultExchangeName,
    const std::string & consumerID,
    HandleMessageCallback_t        onMessageCallback ) :
  _rabbitClient( new RabbitClientImpl ( connectionDetails,
        defaultExchangeName,
        consumerID,
        onMessageCallback ) )
{}

ReturnStatus RabbitClient::start()
{ 
    return _rabbitClient->start(); 
}

ReturnStatus RabbitClient::stop(bool immediate)
{ 
    return _rabbitClient->stop(immediate);
}

ReturnStatus RabbitClient::sendUnicast( const std::string & message, 
        const std::string & destination, 
        const std::string & senderID,
        const std::string & exchangeName )
{ 
    return _rabbitClient->sendMessage(message,destination, senderID, exchangeName,  DeliveryType::Unicast);
}

ReturnStatus RabbitClient::sendMulticast( const std::string & message, 
    const std::string & senderID, 
    const std::string & exchangeName )
{ 
    return _rabbitClient->sendMessage(message, "", senderID, exchangeName, DeliveryType::Multicast);
}

ReturnStatus RabbitClient::bindToSelf( const std::string & key )
{ 
    return _rabbitClient->bind(key, DeliveryType::Unicast); 
}

ReturnStatus RabbitClient::bindToDestination( const std::string & key )
{ 
    return _rabbitClient->bind(key, DeliveryType::Multicast); 
}

ReturnStatus RabbitClient::unbindFromSelf( const std::string & key )
{ 
    return _rabbitClient->unbind(key, DeliveryType::Unicast); 
}

ReturnStatus RabbitClient::unbindFromDestination( const std::string & key )
{ 
    return _rabbitClient->unbind(key, DeliveryType::Multicast); 
}

bool RabbitClient::connected() const
{
  return _rabbitClient->connected();
}

ReturnStatus RabbitClient::declareExchange ( const std::string & exchangeName, unsigned int waitTime )
{
  return _rabbitClient->declareExchange( exchangeName, waitTime );
}
