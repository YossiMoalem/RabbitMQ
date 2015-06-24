#include "AMQPClient.h"
#include "AMQPConnectionHandler.h"

namespace AMQP {

AMQPClient::AMQPClient( OnMessageReveivedCB onMsgReceivedCB ):
    _jobManager( onMsgReceivedCB )
{}

bool AMQPClient::init( const AMQPConnectionDetails & connectionParams )
{
    _connectionParams = connectionParams;
    return _jobManager.start( connectionParams );
}

DeferedResult AMQPClient::publish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message ) const
{
    PostMessage * msg = new PostMessage( exchangeName, 
            routingKey, 
            message );
    return _jobManager.addJob( msg );
}

DeferedResult AMQPClient::bindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    BindMessage * bindMessage = new BindMessage( exchangeName, 
            queueName, 
            routingKey );
    return _jobManager.addJob( bindMessage );
}

DeferedResult AMQPClient::unBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    UnBindMessage * unBindMessage = new UnBindMessage( exchangeName, 
            queueName, 
            routingKey );
    return _jobManager.addJob( unBindMessage );
}

DeferedResult AMQPClient::stop( bool immediate )
{
    StopMessage * stopMessage = new StopMessage( immediate );
    return _jobManager.addJob( stopMessage );
}

DeferedResult AMQPClient::login()
{
    LoginMessage * loginMessage = new LoginMessage( _connectionParams._userName, 
            _connectionParams._password );
    return _jobManager.addJob( loginMessage);
}

DeferedResult AMQPClient::declareExchange( const std::string & exchangeName, 
           ExchangeType exchangetype, 
           bool durable ) const 
{
    DeclareExchangeMessage * declareExchangeMessage = new DeclareExchangeMessage( exchangeName, 
            exchangetype, 
            durable );
    return _jobManager.addJob( declareExchangeMessage );
}

DeferedResult AMQPClient::declareQueue( const std::string & queueName, 
           bool durable,
           bool exclusive, 
           bool autoDelete) const
{
    DeclareQueueMessage * declareQueueMessage = new DeclareQueueMessage( queueName,
            durable,
            exclusive,
            autoDelete );
    return _jobManager.addJob( declareQueueMessage );
}

void AMQPClient::waitForDisconnection()
{
    _jobManager.waitForDisconnection();
}

} //namespace AMQP

