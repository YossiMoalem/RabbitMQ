#include "AMQPConnection.h"
#include "ConnectionDetails.h"
#include <AMQPConnectionDetails.h>
#include <thread>
#include <unistd.h>
#include <future>
#include <chrono>

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
    _isConnected( false ),
    _exchangeName( exchangeName ),
    _queueName( queueName ),
    _routingKey( routingKey )
{}

ReturnStatus AMQPConnection::start()
{
//    _startLoopThread = std::thread( std::bind( &AMQP::AMQPClient::connectLoop, this ) );
    _startLoopThread = std::thread( std::bind( &AMQPConnection::connectLoop, this ) );
    return ReturnStatus::Ok;
}

ReturnStatus AMQPConnection::connectLoop()
{
    _stop = false;
    while ( _stop == false )
    {
        std::cout << "started AMQPConnection::connectLoop() " << std::endl;
        AMQP::AMQPConnectionDetails connectionDetails = _connectionDetails.getNextHost();
        std::thread eventLoopThread = std::thread( std::bind( 
                    &AMQP::AMQPClient::startEventLoop, 
                    &_connectionHandler ) );

        //TODO: currently the login returns false if it cant create socket (e.g internet down)
        //but it will not return false if the credentials are wrong. we should catch it somehow
        std::future_status status;
        if (! _connectionHandler.login( connectionDetails ) )
        {
            std::cout <<"Login failed. Disconnecting..."<<std::endl;
        } else {
            std::future< bool > declareExchangeResult = _connectionHandler.declareExchange( 
                    _exchangeName, 
                    AMQP::topic, 
                    false );
            status = declareExchangeResult.wait_for(std::chrono::seconds(10));
            if( ! ( status == std::future_status::ready && declareExchangeResult.get() ) )
            {
                _connectionHandler.stop( true );
                std::cout << "error declaring exchange" <<std::endl;
            } else {
                std::cout << "exchange declared" <<std::endl;

                // TODO: change to exclusive (_queueName, false, true, false)
                std::future< bool > declareQueueResult = _connectionHandler.declareQueue(
                        _queueName, 
                        false, 
                        false,
                        true );
                status = declareQueueResult.wait_for(std::chrono::seconds(10));
                if( ! ( status == std::future_status::ready && declareQueueResult.get() ) )
                {
                    _connectionHandler.stop( true );
                    std::cout << "error declaring queue" <<std::endl;
                } else {
                    std::cout << "queue declared" <<std::endl;
                    //            std::future< bool > bindResult = _connectionHandler.bindQueue( _exchangeName, _queueName, _routingKey );
                    //            status = bindResult.wait_for(std::chrono::seconds(5));
                    //            if( status == std::future_status::ready && bindResult.get() )
                    //            {
                    //                std::cout << "queue binded" <<std::endl;
                    //            } else {
                    //                std::cout << "error binding queue" <<std::endl;
                    //                sleep( 3 );
                    //                continue;
                    //            }
                    _connectionHandler.bindQueue( _exchangeName, _queueName, _routingKey );
                    rebind();
                    _isConnected = true;
                    std::cout << "CONNECTED" << std::endl;
                    _connectionDetails.reset();
                }
            }
        }
        eventLoopThread.join();
        _isConnected = false;
        std::cout << "DISCONNECTED" << std::endl;
        sleep(1);
    }
    return ReturnStatus::Ok;
}

ReturnStatus AMQPConnection::stop( bool immediate )
{
    _stop = true;
    _connectionHandler.stop( immediate );
    _startLoopThread.join();
    return ReturnStatus::Ok;
}

ReturnStatus AMQPConnection::publish( const std::string & exchangeName, 
        const std::string & routingKey,
        const std::string & message ) const
{
    if ( ! connected() )
    {
        return  ReturnStatus::ClientDisconnected;
    }
    _connectionHandler.publish( _exchangeName, routingKey, message );
    return ReturnStatus::Ok;
}

ReturnStatus AMQPConnection::bind( const std::string & exchangeName,
        const std::string & queueName,
        const std::string routingKey)
{
    std::lock_guard< std::mutex > lock ( _bindingsSetMutex );
    _bindingsSet.insert( routingKey );
    return AMQPConnection::_bind( exchangeName, queueName, routingKey );
}

ReturnStatus AMQPConnection::_bind( const std::string & exchangeName,
        const std::string & queueName,
        const std::string routingKey) const
{
    if ( ! connected() )
    {
        return  ReturnStatus::ClientDisconnected;
    }
    _connectionHandler.bindQueue( exchangeName, queueName, routingKey ); //.get();
    return ReturnStatus::Ok;
}

ReturnStatus AMQPConnection::unBind( const std::string & exchangeName, 
        const std::string & queueName,
        const std::string routingKey)
{
    std::lock_guard< std::mutex > lock ( _bindingsSetMutex );
    _bindingsSet.erase( routingKey );
    return AMQPConnection::_unBind( exchangeName, queueName, routingKey );
}

ReturnStatus AMQPConnection::_unBind( const std::string & exchangeName,
        const std::string & queueName,
        const std::string routingKey) const
{
    if ( ! connected() )
    {
        return  ReturnStatus::ClientDisconnected;
    }
    _connectionHandler.unBindQueue( exchangeName, queueName, routingKey );
    return ReturnStatus::Ok;
}

ReturnStatus AMQPConnection::rebind()
{
    std::lock_guard< std::mutex > lock ( _bindingsSetMutex );
    for ( const std::string& routingKey: _bindingsSet )
        AMQPConnection::_bind( _exchangeName, _queueName, routingKey );
    return ReturnStatus::Ok;
}

bool AMQPConnection::connected() const
{
    return _isConnected;
}
