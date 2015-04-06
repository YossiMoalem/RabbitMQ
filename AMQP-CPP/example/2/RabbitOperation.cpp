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

} //namespace AMQP
