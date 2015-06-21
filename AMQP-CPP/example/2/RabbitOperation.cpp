#include "RabbitOperation.h"
#include "AMQPConnectionHandler.h"

#include "AMQPEventLoop.h"

namespace AMQP {

void PostMessage::handle( )
{
    _handler->doPublish(_exchangeName,
            _routingKey,
            _message,
            _returnValueSetter);
}

void BindMessage::handle( )
{
    _handler->doBindQueue(_exchangeName,
            _queueName,
            _routingKey,
            _returnValueSetter);
}

void UnBindMessage::handle( )
{
    _handler->doUnBindQueue( _exchangeName,
            _queueName,
            _routingKey,
            _returnValueSetter );
}

void StopMessage::handle( )
{
    //TODO:
    //_handler->stop( _immediate );
}

void LoginMessage::handle( )
{
    _handler->login( _userName, _password, _returnValueSetter );
}

void DeclareExchangeMessage::handle( )
{
    _handler->declareExchange( _exchangeName, _exchangeType, _durable, _returnValueSetter );
}

void DeclareQueueMessage::handle( )
{
    _handler->declareQueue( _queueName, _durable, _exclusive, _autoDelete, _returnValueSetter ); 
}

} //namespace AMQP
