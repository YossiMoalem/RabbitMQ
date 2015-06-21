#include "RabbitOperation.h"
#include "AMQPConnectionHandler.h"

#include "AMQPEventLoop.h"

namespace AMQP {

void PostMessage::handle( )
{
    _connectionHandler->doPublish(_exchangeName,
            _routingKey,
            _message,
            _returnValueSetter);
}

void BindMessage::handle( )
{
    _connectionHandler->doBindQueue(_exchangeName,
            _queueName,
            _routingKey,
            _returnValueSetter);
}

void UnBindMessage::handle( )
{
    _connectionHandler->doUnBindQueue( _exchangeName,
            _queueName,
            _routingKey,
            _returnValueSetter );
}

void StopMessage::handle( )
{
    _connectionHandler->stop( _immediate );
}

void LoginMessage::handle( )
{
    _connectionHandler->login( _userName, _password, _returnValueSetter );
}

void DeclareExchangeMessage::handle( )
{
    _connectionHandler->declareExchange( _exchangeName, _exchangeType, _durable, _returnValueSetter );
}

void DeclareQueueMessage::handle( )
{
    _connectionHandler->declareQueue( _queueName, _durable, _exclusive, _autoDelete, _returnValueSetter ); 
}

} //namespace AMQP
