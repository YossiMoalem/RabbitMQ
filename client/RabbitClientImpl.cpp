#include "RabbitClientImpl.h"

RabbitClientImpl::RabbitClientImpl(const ConnectionDetails & i_connectionDetails, 
        const std::string& i_exchangeName, 
        const std::string& i_consumerID,
        CallbackType        i_onMessageCB ) :
    _AMQPConnection( i_connectionDetails, 
            i_exchangeName, 
            i_consumerID, 
            createRoutingKey( i_consumerID, i_consumerID, DeliveryType::Unicast ),
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
    std::string routingKey = createRoutingKey( i_senderID, i_destination, i_deliveryType);
    std::string serializedMessage = serializePostMessage( i_senderID, i_destination,i_deliveryType, i_message );
    return _AMQPConnection.publish( _exchangeName, routingKey, serializedMessage );
}

ReturnStatus RabbitClientImpl::bind(const std::string& i_key, DeliveryType i_deliveryType)
{ 
    std::string routingKey = createRoutingKey( i_key, i_key, i_deliveryType );
    return _AMQPConnection.bind( _exchangeName, _queueName, routingKey );
}

ReturnStatus RabbitClientImpl::unbind(const std::string& i_key, DeliveryType i_deliveryType)
{ 
    std::string routingKey = createRoutingKey( i_key, i_key, i_deliveryType );
  return _AMQPConnection.unBind( _exchangeName, _queueName, routingKey );
}

bool RabbitClientImpl::connected() const
{
  return _AMQPConnection.connected();
}

 int RabbitClientImpl::onMessageReceived( const AMQP::Message & message ) const
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

std::string RabbitClientImpl::createRoutingKey( const std::string & sender, 
                    const std::string & destination,
                    DeliveryType deliveryType )const
{
    static const char* const UNICAST_PREFIX = "ALL:";
    static const char* const MULTICAST_SUFFIX =":ALL";

    std::string routingKey;
    if( deliveryType == DeliveryType::Unicast )
        routingKey = UNICAST_PREFIX + destination;
    else
        routingKey = sender + MULTICAST_SUFFIX;
    return routingKey;

}
