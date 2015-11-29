#include "RabbitClientImpl.h"
#include "CallbackHandler.h"

RabbitClientImpl::RabbitClientImpl(const ConnectionDetails & _connectionDetails, 
        const std::string& _exchangeName, 
        const std::string& _lucExchangeName,
        const std::string& _consumerID,
        CallbackType        _onMessageCB ) :
    _AMQPConnection( _connectionDetails, 
            _exchangeName,
            _lucExchangeName,
            _consumerID, 
            "*",
            //createRoutingKey( _consumerID, _consumerID, DeliveryType::Unicast ),
            [ this ] ( const AMQP::Message & message ) {  return onMessageReceived( message ); } ),
    _exchangeName(_exchangeName),
    _lucExchangeName(_lucExchangeName),
    _queueName(_consumerID),
    _callbackHandler( _onMessageCB )
    //_onMessageReceivedCB( _onMessageCB )
{
    _callbackHandler.start();
}

ReturnStatus RabbitClientImpl::start()
{
    return _AMQPConnection.start();
}

ReturnStatus RabbitClientImpl::stop( bool immediate )
{
    return _AMQPConnection.stop( immediate );
}

//use default exchange
ReturnStatus RabbitClientImpl::sendMessage(const std::string& _message,
        const std::string& _destination,
        const std::string& _senderID,
        DeliveryType _deliveryType) const
{
    return sendMessage( _message, _destination, _senderID, _exchangeName, _deliveryType );
}

ReturnStatus RabbitClientImpl::sendMessage(const std::string& _message,
        const std::string& _destination, 
        const std::string& _senderID,
        const std::string& _excName,
        DeliveryType _deliveryType) const
{
    std::string routingKey = createRoutingKey( _senderID, _destination, _deliveryType);
    std::string serializedMessage = serializePostMessage( _senderID, _destination,_deliveryType, _message );
    return _AMQPConnection.publish( _excName, routingKey, serializedMessage );
}

ReturnStatus RabbitClientImpl::bind(const std::string& _key, DeliveryType _deliveryType)
{ 
    std::string routingKey = createRoutingKey( _key, _key, _deliveryType );
    return _AMQPConnection.bind( _exchangeName, _queueName, routingKey );
}

ReturnStatus RabbitClientImpl::unbind(const std::string& _key, DeliveryType _deliveryType)
{ 
    std::string routingKey = createRoutingKey( _key, _key, _deliveryType );
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
    _callbackHandler.addMessage( sender, destination, deliveryType, text );
    return 0;
   // return  _onMessageReceivedCB( sender, destination, deliveryType, text );
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
