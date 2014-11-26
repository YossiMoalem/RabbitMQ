#include "simpleClient.h"
#include "clientImpl.h"

simpleClient::simpleClient(const connectionDetails& i_connectionDetails, 
    const std::string& i_exchangeName, 
    const std::string& i_consumerID,
    RabbitMQNotifiableIntf* i_handler) :
  p_impl( new RabbitClientImpl (i_connectionDetails,
        i_exchangeName,
        i_consumerID,
        i_handler))
{}

simpleClient::simpleClient(const connectionDetails& i_connectionDetails, 
    const std::string& i_exchangeName, 
    const std::string& i_consumerID,
    int (*i_onMessageCB)(AMQPMessage*) ):
  p_impl( new RabbitClientImpl (i_connectionDetails,
        i_exchangeName,
        i_consumerID,
        i_onMessageCB ) )
{}

int simpleClient::start()                  { return p_impl->start(); }
int simpleClient::stop(bool immediate)     { return p_impl->stop(immediate);}

int simpleClient::sendUnicast(const std::string& i_message, const std::string& i_destination)
{ return p_impl->sendUnicast (i_message,i_destination); }
int simpleClient::sendMulticast(const std::string& i_message, const std::string& i_destination)
{ return p_impl->sendMulticast(i_message,i_destination); }

int simpleClient::bindToSelf(const std::string& i_key)
{ return p_impl->bindToSelf(i_key); }
int simpleClient::bindToDestination(const std::string& i_key)
{ return p_impl->bindToDestination(i_key); }
int simpleClient::unbindFromSelf(const std::string& i_key)
{ return p_impl->unbindFromSelf(i_key); }
int simpleClient::unbindFromDestination(const std::string& i_key)
{ return p_impl->unbindFromDestination(i_key); }

