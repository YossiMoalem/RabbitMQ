#include "AMQPClient.h"
#include "AmqpConnectionDetails.h"
#include "AMQPConnectionHandler.h"
#include "AMQPEventLoop.h"

namespace AMQP {

AMQPClient::AMQPClient( OnMessageReveivedCB onMsgReceivedCB ):
    _eventLoop( new AMQPEventLoop(onMsgReceivedCB,  & _jobQueue ) )
{}

AMQPClient::~AMQPClient()
{
    delete _eventLoop;
}

int AMQPClient::startEventLoop()
{
    return _eventLoop->start();
}

std::future< bool > AMQPClient::publish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message ) const
{
    PostMessage * msg = new PostMessage( exchangeName, 
            routingKey, 
            message );
    _jobQueue.push( msg );
    return msg->deferedResult();
}

std::future< bool > AMQPClient::bindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    BindMessage * bindMessage = new BindMessage( exchangeName, 
            queueName, 
            routingKey );
    _jobQueue.push( bindMessage );
    return bindMessage->deferedResult();
}

std::future< bool > AMQPClient::unBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    UnBindMessage * unBindMessage = new UnBindMessage( exchangeName, 
            queueName, 
            routingKey );
    _jobQueue.push( unBindMessage );
    return unBindMessage->deferedResult();
}


bool AMQPClient::login( const AmqpConnectionDetails & connectionParams )
{
    return _eventLoop->connectionHandler()->login( connectionParams );
}

std::future< bool > AMQPClient::declareExchange( const std::string & exchangeName, 
           ExchangeType type , 
           bool durable ) const 
{
    return _eventLoop->connectionHandler()->declareExchange( exchangeName, type, durable );
}

std::future< bool > AMQPClient::declareQueue( const std::string & queueName, 
           bool durable,
           bool exclusive, 
           bool autoDelete) const
{
    return _eventLoop->connectionHandler()->declareQueue( queueName, durable, exclusive, autoDelete );
}
} //namespace AMQP

