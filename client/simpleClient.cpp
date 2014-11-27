#include "simpleClient.h"
#include "clientImpl.h"

simpleClient::simpleClient(const connectionDetails& i_connectionDetails, 
    const std::string& i_exchangeName, 
    const std::string& i_consumerID,
    ExchangeType       i_exchangeType,
    RabbitMQNotifiableIntf* i_handler) :
  p_impl( new RabbitClientImpl (i_connectionDetails,
        i_exchangeName,
        i_consumerID,
        i_exchangeType,
        i_handler))
{}

simpleClient::simpleClient(const connectionDetails& i_connectionDetails, 
    const std::string& i_exchangeName, 
    const std::string& i_consumerID,
    ExchangeType       i_exchangeType,
    int (*i_onMessageCB)(AMQPMessage*) ):
  p_impl( new RabbitClientImpl (i_connectionDetails,
        i_exchangeName,
        i_consumerID,
        i_exchangeType,
        i_onMessageCB ) )
{}

int simpleClient::start()                  { return p_impl->start(); }
int simpleClient::stop(bool immediate)     { return p_impl->stop(immediate);}

int simpleClient::sendUnicast(const std::string& i_message, const std::string& i_destination, const std::string& i_senderID)
{ 
    return p_impl->sendMessage(i_message,i_destination, i_senderID, DeliveryType::Unicast); 
}
int simpleClient::sendMulticast(const std::string& i_message, const std::string& i_destination, const std::string& i_senderID)
{ 
    return p_impl->sendMessage(i_message,i_destination, i_senderID, DeliveryType::Multicast); 
}


int simpleClient::bindToSelf(const std::string& i_key)
{ 
    return p_impl->bind(i_key, DeliveryType::Unicast); 
}
int simpleClient::bindToDestination(const std::string& i_key)
{ 
    return p_impl->bind(i_key, DeliveryType::Multicast); 
}
int simpleClient::unbindFromSelf(const std::string& i_key)
{ 
    return p_impl->unbind(i_key, DeliveryType::Unicast); 
}
int simpleClient::unbindFromDestination(const std::string& i_key)
{ 
    return p_impl->unbind(i_key, DeliveryType::Multicast); 
}

