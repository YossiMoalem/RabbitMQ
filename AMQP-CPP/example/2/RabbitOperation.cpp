#include "RabbitOperation.h"
#include "AMQPConnectionHandler.h"

#include "AMQPEventLoop.h"

namespace AMQP {

void PostMessage::handle( AMQPEventLoop * eventLoop )
{
    eventLoop->publish( exchangeName(), 
            routingKey(), 
            message(), 
            resultSetter() );
}

void BindMessage::handle( AMQPEventLoop * eventLoop )
{
    eventLoop->bindQueue(exchangeName(), 
            queueName(), 
            routingKey(), 
            resultSetter() ); 
}

void UnBindMessage::handle( AMQPEventLoop * eventLoop )
{
    eventLoop->unBindQueue(exchangeName(), 
            queueName(), 
            routingKey(), 
            resultSetter() ); 
}

void StopMessage::handle( AMQPEventLoop * eventLoop )
{
    eventLoop->stop( immediate ); 
}

void LoginMessage::handle( AMQPEventLoop * eventLoop )
{
    eventLoop->login( _userName, _password, resultSetter() );
}

void DeclareExchangeMessage::handle( AMQPEventLoop * eventLoop )
{
    eventLoop->declareExchange( _exchangeName, _exchangeType, _durable, resultSetter() );
}

void DeclareQueueMessage::handle( AMQPEventLoop * eventLoop )
{
    eventLoop->declareQueue( _queueName, _durable, _exclusive, _autoDelete, resultSetter() ); 
}

} //namespace AMQP
