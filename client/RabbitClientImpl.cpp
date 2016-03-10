#include "RabbitClientImpl.h"

RabbitClientImpl::RabbitClientImpl( const ConnectionDetails & connectionDetails, 
        const std::string & defaultExchangeName, 
        const std::string & consumerID,
        HandleMessageCallback_t onMessageCallback ) :
    _AMQPConnection( connectionDetails, 
            _exchangesName,
            consumerID, 
            "*",
            [ this ] ( const AMQP::Message & message ) {  return onMessageReceived( message ); } ),
    _queueName( consumerID ),
    _receivedMessageHandler( onMessageCallback )
{
    _exchangesName.push_back( defaultExchangeName );
    _receivedMessageHandler.start();
}

ReturnStatus RabbitClientImpl::start()
{
    return _AMQPConnection.start();
}

ReturnStatus RabbitClientImpl::stop( bool immediate )
{
    return _AMQPConnection.stop( immediate );
}

ReturnStatus RabbitClientImpl::sendMessage(const std::string & message,
        const std::string & destination,
        const std::string & senderID,
        DeliveryType deliveryType) const
{
    return sendMessage( message, destination, senderID, defaultExchangeName(), deliveryType );
}

ReturnStatus RabbitClientImpl::sendMessage(const std::string & message,
        const std::string & destination, 
        const std::string & senderID,
        const std::string & exchangeName,
        DeliveryType deliveryType) const
{
    std::string routingKey = createRoutingKey( senderID, destination, deliveryType );
    std::string serializedMessage = serializePostMessage( senderID, destination, deliveryType, message );
    return _AMQPConnection.publish( exchangeName, routingKey, serializedMessage );
}

ReturnStatus RabbitClientImpl::bind(const std::string & key, DeliveryType deliveryType)
{ 
    std::string routingKey = createRoutingKey( key, key, deliveryType );
    return _AMQPConnection.bind( defaultExchangeName(), _queueName, routingKey );
}

ReturnStatus RabbitClientImpl::unbind(const std::string & key, DeliveryType deliveryType)
{ 
    std::string routingKey = createRoutingKey( key, key, deliveryType );
    return _AMQPConnection.unBind( defaultExchangeName(), _queueName, routingKey );
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
    _receivedMessageHandler.addMessage( sender, destination, deliveryType, text );
    return 0;
}

std::string RabbitClientImpl::serializePostMessage( const std::string & sender,
        const std::string & destination,
        DeliveryType deliveryType,
        const std::string & message)
{
    std::stringstream ss;
    ss << ( int ) deliveryType << std::endl
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
    std::string deliveryTypeAsString;
    std::istringstream is( serializedMessage );
    getline( is, deliveryTypeAsString );
    getline( is, sender );
    getline( is, destination );
    int messageStart = is.tellg();
    message = (is.str()).substr( messageStart );
    int deliveryTypeAsInt = boost::lexical_cast<int>( deliveryTypeAsString );
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


ReturnStatus RabbitClientImpl::declareExchange ( const std::string & exchangeName, unsigned int waitTime )
{
    bool exchangeAdded =  _AMQPConnection.declareExchange( exchangeName, waitTime );
    if ( exchangeAdded )
        _exchangesName.push_back( exchangeName );
    return exchangeAdded ? ReturnStatus::Ok : ReturnStatus::OperationFailed;
}

const std::string & RabbitClientImpl::defaultExchangeName() const
{
    return _exchangesName[ 0 ];
}
