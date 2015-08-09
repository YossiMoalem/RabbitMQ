#include "RabbitOperation.h"
#include "RabbitJobManager.h"
#include "Debug.h"
#include "AMQPEventLoop.h"

namespace AMQP {

void PostMessage::handle( )
{
    _handler->connectionHandler()->doPublish(_exchangeName,
            _routingKey,
            _message,
            _returnValueSetter);
}

void BindMessage::handle( )
{
    _handler->connectionHandler()->doBindQueue(_exchangeName,
            _queueName,
            _routingKey,
            _returnValueSetter);
}

void UnBindMessage::handle( )
{
    _handler->connectionHandler()->doUnBindQueue( _exchangeName,
            _queueName,
            _routingKey,
            _returnValueSetter );
}

void StopMessage::handle( )
{
    PRINT_DEBUG(DEBUG, "Handle stop message");
    _handler->stopEventLoop( _immediate,
            _returnValueSetter );
}

void LoginMessage::handle( )
{
    _handler->connectionHandler()->login( _userName, _password, _returnValueSetter );
}

void DeclareExchangeMessage::handle( )
{
    _handler->connectionHandler()->declareExchange( _exchangeName, 
            _exchangeType, 
            _durable, 
            _returnValueSetter );
}

void DeclareQueueMessage::handle( )
{
    _handler->connectionHandler()->declareQueue( _queueName, 
            _durable, 
            _exclusive, 
            _autoDelete, 
            _returnValueSetter ); 
}

} //namespace AMQP
