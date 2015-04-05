#include "AMQPClient.h"
#include "AmqpConnectionDetails.h"
#include "AMQPConnectionHandler.h"
#include "RabbitMessage.h"
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
    AMQPConnectionHandler::OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    PostMessage * msg = new PostMessage( exchangeName, 
            routingKey, 
            message, 
            operationSucceeded );
    _jobQueue.push( msg );
    return operationSucceeded->get_future();
}

std::future< bool > AMQPClient::bindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    AMQPConnectionHandler::OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    BindMessage * bindMessage = new BindMessage( exchangeName, 
            queueName, 
            routingKey, 
            operationSucceeded );
    _jobQueue.push( bindMessage );
    return operationSucceeded->get_future();
}

std::future< bool > AMQPClient::unBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    AMQPConnectionHandler::OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    UnBindMessage * unBindMessage = new UnBindMessage( exchangeName, 
            queueName, 
            routingKey, 
            operationSucceeded );
    _jobQueue.push( unBindMessage );
    return operationSucceeded->get_future();
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

