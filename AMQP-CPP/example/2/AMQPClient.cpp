#include "AMQPClient.h"
#include "AMQPConnectionDetails.h"
#include "AMQPConnectionHandler.h"

namespace AMQP {

AMQPClient::AMQPClient( OnMessageReveivedCB onMsgReceivedCB ):
    _jobManager( onMsgReceivedCB )
{}

int AMQPClient::connect( const AMQPConnectionDetails & connectionParams )
{
    bool connected = _jobManager.connect( connectionParams );
    if ( connected )
    {
        _jobManager.startEventLoop();
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

DeferedResult AMQPClient::login( const AMQPConnectionDetails & connectionParams )
{
    //TODO: wait (?) for event loop to start, or indicate it is not started in some way
    LoginMessage * loginMessage = new LoginMessage( connectionParams._userName, 
            connectionParams._password );
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
} //namespace AMQP

