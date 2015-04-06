#include "AMQPConnection.h"
#include "ConnectionDetails.h"

#include <AmqpConnectionDetails.h>

#include <thread>

AMQPConnection::AMQPConnection( const ConnectionDetails & connectionDetails,
        const std::string & exchangeName ,
        const std::string & queueName,
        const std::string & routingKey,
        AMQP::AMQPClient::OnMessageReveivedCB i_onMessageReceiveCB ) :
    _connectionHandler( [ i_onMessageReceiveCB ] 
            ( const AMQP::Message & message ) 
            { return i_onMessageReceiveCB( message ); } ),
    _connectionDetails( connectionDetails ),
    _stop( false ),
    _exchangeName( exchangeName ),
    _queueName( queueName ),
    _routingKey( routingKey )
{}

ReturnStatus AMQPConnection::start()
{
    //TODO: handle disconnection:
    //1. when the thread exits - get new host, 
    //1.1 login
    //1.2 re-start event loop
    //1.3 rebind
    _stop = false;
    AMQP::AmqpConnectionDetails connectionDetails = _connectionDetails.getFirstHost();
    _connectionHandler.login( connectionDetails );
    _eventLoopThread = std::thread( std::bind( &AMQP::AMQPClient::startEventLoop, &_connectionHandler ) );
    _connectionHandler.declareExchange( _exchangeName, AMQP::topic, false );
    _connectionHandler.declareQueue( _queueName );
    //TODO: WAIT! check retvals!

    _connectionHandler.bindQueue( _exchangeName, _queueName, _routingKey );
    //TODO: WAIT! check retvals!

    return ReturnStatus::Ok;
}

ReturnStatus AMQPConnection::stop( bool immediate )
{
    _stop = true;
    _connectionHandler.stop( immediate );
    _eventLoopThread.join();
    return ReturnStatus::Ok;
}

void AMQPConnection::publish( const std::string & exchangeName, 
        const std::string & routingKey,
        const std::string & message ) const
{
    _connectionHandler.publish( _exchangeName, routingKey, message );
}

ReturnStatus AMQPConnection::bind( const std::string & exchangeName,
        const std::string & queueName,
        const std::string routingKey) const
{
    _connectionHandler.bindQueue( exchangeName, queueName, routingKey );
    return ReturnStatus::Ok;
}

ReturnStatus AMQPConnection::unBind( const std::string & exchangeName, 
        const std::string & queueName,
        const std::string routingKey) const
{
    _connectionHandler.unBindQueue( exchangeName, queueName, routingKey );
    return ReturnStatus::Ok;
}

bool AMQPConnection::connected() const
{
    //implement
    return true;
}
