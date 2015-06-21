#include "AMQPClient.h"
#include "AMQPConnectionDetails.h"
#include "AMQPConnectionHandler.h"

namespace AMQP {

AMQPClient::AMQPClient( OnMessageReveivedCB onMsgReceivedCB ):
    _connectionHandler( new AMQPConnectionHandler(onMsgReceivedCB,  _jobQueue ) )
{}

AMQPClient::~AMQPClient()
{
    delete _connectionHandler;
}

int AMQPClient::connect( const AMQPConnectionDetails & connectionParams )
{
    bool connected = _connectionHandler->connect( connectionParams );
    if ( connected )
    {
        _connectionHandler->startEventLoop();
        return 0;
    }
    return 3;
}

DeferedResult AMQPClient::publish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message ) const
{
    PostMessage * msg = new PostMessage( exchangeName, 
            routingKey, 
            message, 
            _connectionHandler);
    auto result = msg->deferedResult();
    _jobQueue.push( msg );
    return result;
}

DeferedResult AMQPClient::bindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    BindMessage * bindMessage = new BindMessage( exchangeName, 
            queueName, 
            routingKey, 
            _connectionHandler);
    auto result = bindMessage->deferedResult();
    _jobQueue.push( bindMessage );
    return result;
}

DeferedResult AMQPClient::unBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    UnBindMessage * unBindMessage = new UnBindMessage( exchangeName, 
            queueName, 
            routingKey, 
            _connectionHandler);
    auto result = unBindMessage->deferedResult();
    _jobQueue.push( unBindMessage );
    return result;
}

DeferedResult AMQPClient::stop( bool immediate )
{
    StopMessage * stopMessage = new StopMessage( immediate, 
            _connectionHandler);
    _jobQueue.pushFront( stopMessage );
    return stopMessage->deferedResult();
}

DeferedResult AMQPClient::login( const AMQPConnectionDetails & connectionParams )
{
    //TODO: wait (?) for event loop to start, or indicate it is not started in some way
    LoginMessage * loginMessage = new LoginMessage( connectionParams._userName, connectionParams._password, 
            _connectionHandler);
    auto result = loginMessage->deferedResult();
    _jobQueue.pushFront( loginMessage );
    return result;
}

DeferedResult AMQPClient::declareExchange( const std::string & exchangeName, 
           ExchangeType exchangetype, 
           bool durable ) const 
{
    DeclareExchangeMessage * declareExchangeMessage = new DeclareExchangeMessage( exchangeName, 
            exchangetype, 
            durable, 
            _connectionHandler);
    auto result = declareExchangeMessage->deferedResult(); 
    _jobQueue.pushFront( declareExchangeMessage );
    return result;
}

DeferedResult AMQPClient::declareQueue( const std::string & queueName, 
           bool durable,
           bool exclusive, 
           bool autoDelete) const
{
    DeclareQueueMessage * declareQueueMessage = new DeclareQueueMessage( queueName, durable, exclusive, autoDelete, 
            _connectionHandler);
    auto result = declareQueueMessage->deferedResult();
    _jobQueue.pushFront( declareQueueMessage );
    return result;
}
} //namespace AMQP

