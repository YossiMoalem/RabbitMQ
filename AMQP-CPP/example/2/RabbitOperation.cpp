#include "RabbitOperation.h"
#include "RabbitJobHandler.h"
#include "Debug.h"

namespace AMQP {

void PostMessage::handle( )
{
    _jobHandler->connectionHandler()->doPublish(_exchangeName,
            _routingKey,
            _message,
            _returnValueSetter);
}

void BindMessage::handle( )
{
    _jobHandler->connectionHandler()->doBindQueue(_exchangeName,
            _queueName,
            _routingKey,
            _returnValueSetter);
}

void UnBindMessage::handle( )
{
    _jobHandler->connectionHandler()->doUnBindQueue( _exchangeName,
            _queueName,
            _routingKey,
            _returnValueSetter );
}

void StopMessage::handle( )
{
    PRINT_DEBUG(DEBUG, "Handle stop message");
    _jobHandler->stopEventLoop( _returnValueSetter );
}

void LoginMessage::handle( )
{
    _jobHandler->connectionHandler()->login( _userName, _password, _returnValueSetter );
}

void DeclareExchangeMessage::handle( )
{
    _jobHandler->connectionHandler()->declareExchange( _exchangeName, 
            _exchangeType, 
            _durable, 
            _returnValueSetter );
}

void DeclareQueueMessage::handle( )
{
    _jobHandler->connectionHandler()->declareQueue( _queueName, 
            _durable, 
            _exclusive, 
            _autoDelete, 
            _returnValueSetter ); 
}

} //namespace AMQP
