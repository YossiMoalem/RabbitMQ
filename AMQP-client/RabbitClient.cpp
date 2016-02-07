#include "RabbitClient.h"
#include "RabbitOperation.h"

namespace AMQP {

RabbitClient::RabbitClient( OnMessageReceivedCB onMsgReceivedCB ):
    _jobQueue(),
    _jobHandler( onMsgReceivedCB, _jobQueue )
{
    _jobQueue.setHandler( &_jobHandler );
}

bool RabbitClient::init( const RabbitConnectionDetails & connectionParams )
{
    _connectionParams = connectionParams;
    return _jobHandler.start( connectionParams );
}

DeferedResult RabbitClient::login() const
{
    LoginMessage * loginMessage = new LoginMessage( _connectionParams._userName, 
            _connectionParams._password );
    return _jobQueue.addJob( loginMessage);
}

DeferedResult RabbitClient::publish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message ) const
{
    PostMessage * msg = new PostMessage( exchangeName, 
            routingKey, 
            message );
    return _jobQueue.addJob( msg );
}

DeferedResult RabbitClient::bindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    BindMessage * bindMessage = new BindMessage( exchangeName, 
            queueName, 
            routingKey );
    return _jobQueue.addJob( bindMessage );
}

DeferedResult RabbitClient::unBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey) const
{
    UnBindMessage * unBindMessage = new UnBindMessage( exchangeName, 
            queueName, 
            routingKey );
    return _jobQueue.addJob( unBindMessage );
}

DeferedResult RabbitClient::stop( bool immediate ) const
{
    StopMessage * stopMessage = new StopMessage( );
    if (immediate)
    {
        return _jobQueue.addJobToFront( stopMessage );
    }
    else
    {
        return _jobQueue.addJob( stopMessage );
    }
}

DeferedResult RabbitClient::declareExchange( const std::string & exchangeName, 
           ExchangeType exchangetype, 
           bool durable ) const 
{
    DeclareExchangeMessage * declareExchangeMessage = new DeclareExchangeMessage( exchangeName, 
            exchangetype, 
            durable );
    return _jobQueue.addJob( declareExchangeMessage );
}

DeferedResult RabbitClient::declareQueue( const std::string & queueName, 
           bool durable,
           bool exclusive, 
           bool autoDelete) const
{
    DeclareQueueMessage * declareQueueMessage = new DeclareQueueMessage( queueName,
            durable,
            exclusive,
            autoDelete );
    return _jobQueue.addJob( declareQueueMessage );
}

DeferedResult RabbitClient::removeQueue( const std::string & queueName ) const
{
    RemoveQueueMessage * removeQueueMessage = new RemoveQueueMessage( queueName );
    return _jobQueue.addJob( removeQueueMessage );
}

void RabbitClient::waitForDisconnection() const
{
    _jobHandler.waitForDisconnection();
}

} //namespace AMQP

