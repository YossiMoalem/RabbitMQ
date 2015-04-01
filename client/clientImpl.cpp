#include "clientImpl.h"
#include <boost/ref.hpp>
#include <signal.h>

#define UNICAST_PREFIX "ALL:"
#define MULTICAST_SUFFIX ":ALL"

RabbitClientImpl::RabbitClientImpl(const ConnectionDetails & i_connectionDetails, 
        const std::string& i_exchangeName, 
        const std::string& i_consumerID,
        CallbackType        i_onMessageCB ) :
    _AMQPConnection( i_connectionDetails, i_exchangeName, i_consumerID, UNICAST_PREFIX + i_consumerID, 
    [ this ] ( const AMQP::Message & message ) {  return onMessageReceived( message ); } ),
    _exchangeName(i_exchangeName),
    _queueName(i_consumerID),
    _onMessageReceivedCB( i_onMessageCB )
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
        DeliveryType i_deliveryType) const
{
    std::string routingKey;
    if( i_deliveryType == DeliveryType::Unicast )
        routingKey = UNICAST_PREFIX + i_destination;
    else
        routingKey = i_senderID + MULTICAST_SUFFIX;
    std::string serializedMessage = serializePostMessage( i_senderID, i_destination,i_deliveryType, i_message );
    _AMQPConnection.publish( _exchangeName, routingKey, serializedMessage );
    //PostMessage* newMessage = new PostMessage(i_message, i_destination, i_senderID, i_deliveryType );
    //return sendMessage(newMessage);
    return ReturnStatus::Ok;
}

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

 int RabbitClientImpl::onMessageReceived( const AMQP::Message & message ) 
{
    std::string  sender;
    std::string  destination;
    DeliveryType deliveryType;
    std::string  text;
    deserializePostMessage( message.message(), sender,destination,deliveryType, text );
    return  _onMessageReceivedCB( sender, destination, deliveryType, text );
}
std::string RabbitClientImpl::serializePostMessage( const std::string & sender,
        const std::string & destination,
        DeliveryType deliveryType,
        const std::string & message)
{
    std::stringstream ss;
    ss << ( int ) deliveryType 
        << sender << std::endl
        << destination << std::endl
        << message ;
    return ss.str();
}

void RabbitClientImpl::deserializePostMessage( const std::string serializedMessage, 
           std::string & sender,
           std::string & destination,
           DeliveryType & deliveryType,
           std::string & message)
{
    int deliveryTypeAsInt;
    std::istringstream is( serializedMessage );
    is >>deliveryTypeAsInt;
    getline( is, sender );
    getline( is, destination );
    int messageStart = is.tellg();
    message = (is.str()).substr( messageStart );
    deliveryType = static_cast<DeliveryType>( deliveryTypeAsInt );
}
