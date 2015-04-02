#include "AMQPClient.h"
#include "AmqpConnectionDetails.h"
#include "AMQPConnection.h"
#include "RabbitMessage.h"

namespace AMQP {

AMQPClient::AMQPClient( AMQPConnection::OnMessageReveivedCB onMsgReceivedCB ):
    _AMQPConnection( new AMQPConnection( onMsgReceivedCB ) )
{}

AMQPClient::~AMQPClient()
{
    delete _AMQPConnection;
}

std::future< bool > AMQPClient::publish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message ) const
{
    AMQPConnection::OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    PostMessage * msg = new PostMessage( exchangeName, routingKey, message, operationSucceeded );

    _jobQueue.push( msg );
    return operationSucceeded->get_future();
}

std::future< bool > AMQPClient::bindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    AMQPConnection::OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    BindMessage * bindMessage = new BindMessage( exchangeName, queueName, routingKey, operationSucceeded );
    _jobQueue.push( bindMessage );
    return operationSucceeded->get_future();
}

std::future< bool > AMQPClient::unBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    AMQPConnection::OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    UnBindMessage * unBindMessage = new UnBindMessage( exchangeName, queueName, routingKey, operationSucceeded );
    _jobQueue.push( unBindMessage );
    return operationSucceeded->get_future();
}


int AMQPClient::startEventLoop()
{
//    if (!_connected )
//        return 1;

    while( true )
    {
        _AMQPConnection->handleInput();
//        if ( !_outgoingMessages.empty() )
//        {
//            _socket.send( _outgoingMessages );
//        }
        handleQueue();
    }
    return 0;
}

void AMQPClient::handleQueue( )
{
    RabbitMessageBase * msg = nullptr;
    if( _jobQueue.try_pop( msg ) )
    {
        switch( msg->messageType() )
        {
            case MessageType::Post:
                {
                    PostMessage * postMessage = static_cast< PostMessage* >( msg );
                    _AMQPConnection->doPublish( postMessage->exchangeName(), 
                            postMessage->routingKey(), 
                            postMessage->message(), 
                            postMessage->resultSetter() );
                    delete msg;
                }
                break;
            case MessageType::Bind:
                {
                    BindMessage * bindMessage = static_cast< BindMessage* >( msg );
                    _AMQPConnection->doBindQueue(bindMessage->exchangeName(), 
                            bindMessage->queueName(), 
                            bindMessage->routingKey(), 
                            bindMessage->resultSetter() ); 
                }
                break;
            case MessageType::UnBind:
                {
                    UnBindMessage * unBindMessage = static_cast< UnBindMessage* >( msg );
                    _AMQPConnection->doUnBindQueue(unBindMessage->exchangeName(), 
                            unBindMessage->queueName(), 
                            unBindMessage->routingKey(), 
                            unBindMessage->resultSetter() ); 
                }
                break;
        }
    }
}

bool AMQPClient::login( const AmqpConnectionDetails & connectionParams )
{
    return _AMQPConnection->login( connectionParams );
}
std::future< bool > AMQPClient::declareExchange( const std::string & exchangeName, 
           ExchangeType type , 
           bool durable ) const 
{
    return _AMQPConnection->declareExchange( exchangeName, type, durable );
}
std::future< bool > AMQPClient::declareQueue( const std::string & queueName, 
           bool durable,
           bool exclusive, 
           bool autoDelete) const
{
    return _AMQPConnection->declareQueue( queueName, durable, exclusive, autoDelete );
}
} //namespace AMQP

