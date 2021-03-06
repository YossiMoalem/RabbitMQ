#include "AMQPConnection.h"
#include "ConnectionDetails.h"
#include "Debug.h"
#include <Types.h>
#include <thread>
#include <unistd.h>
#include <future>
#include <chrono>

#define MAX_WAIT_TIME_FOR_ANSWER_IN_SEC 10

AMQPConnection::AMQPConnection( const ConnectionDetails & connectionDetails,
        const std::vector< std::string > & exchangesName ,
        const std::string & queueName,
        const std::string & routingKey,
        AMQP::OnMessageReceivedCB i_onMessageReceiveCB ) :
    _connectionHandler( [ i_onMessageReceiveCB ]
            ( const AMQP::Message & message )
            { return i_onMessageReceiveCB( message ); } ),
    _connectionDetails( connectionDetails ),
    _stop( false ),
    _isConnected( false ),
    _exchangesName( exchangesName ),
    _queueName( queueName ),
    _routingKey( routingKey )
{}

ReturnStatus AMQPConnection::start()
{
    _startLoopThread = std::thread( std::bind( &AMQPConnection::connectLoop, this ) );
    return ReturnStatus::Ok;
}

ReturnStatus AMQPConnection::connectLoop()
{
    PRINT_DEBUG(DEBUG, "started connectLoop thread");
    _stop = false;
    while ( _stop == false )
    {
        PRINT_DEBUG(DEBUG, "started AMQPConnection::connectLoop() ");
        AMQP::RabbitConnectionDetails connectionDetails = _connectionDetails.nextHost();

        bool connected = _connectionHandler.init( connectionDetails );
        if ( connected )
        {
            bool logginSucceeded = _login();
            if ( ! logginSucceeded )
            {
                PRINT_DEBUG(DEBUG, "Login failed. Disconnecting...");
                _connectionHandler.stop( true );
            } else {
                bool declareExchangeSucceeded = _declareExchanges();
                if ( ! declareExchangeSucceeded )
                {
                    PRINT_DEBUG(DEBUG, "error declaring exchange");
                    _connectionHandler.stop( true );
                } else {
                    bool removeQueueSucceeded = _removeQueue();
                    if ( ! removeQueueSucceeded )
                    {
                        PRINT_DEBUG(DEBUG, "error removing queue");
                        _connectionHandler.stop( true );
                    } else {
                        bool declareQueueSucceeded = _declareQueue();
                        if ( ! declareQueueSucceeded )
                        {
                            PRINT_DEBUG(DEBUG, "error declaring queue");
                            _connectionHandler.stop( true );
                        } else {
                            _isConnected = true;
                            rebind();
                            PRINT_DEBUG(DEBUG, "CONNECTED");
                            _connectionDetails.reset();
                        }
                    }
                }
            }
            _connectionHandler.waitForDisconnection(); 
        }
        _isConnected = false;
        PRINT_DEBUG(DEBUG, "DISCONNECTED");
        sleep(2);
    }
    PRINT_DEBUG(DEBUG, "exit connectLoop thread");
    return ReturnStatus::Ok;
}

bool AMQPConnection::declareExchange ( const std::string & exchangeName, unsigned int waitTime ) const
{
    std::future< bool > declareExchangeResult = _connectionHandler.declareExchange( 
            exchangeName,
            AMQP::topic,
            false );
    std::future_status status = declareExchangeResult.wait_for(std::chrono::seconds( waitTime ));
    if( status != std::future_status::ready )
    {
        PRINT_DEBUG(DEBUG, "Did not get answer in time. Consider it as failure");
    }
    return ( status == std::future_status::ready && declareExchangeResult.get() );
}

bool AMQPConnection::_login() const
{
    std::future< bool > loginStatus = _connectionHandler.login();
    std::future_status status = loginStatus.wait_for(std::chrono::seconds( MAX_WAIT_TIME_FOR_ANSWER_IN_SEC ));
    if( status != std::future_status::ready )
    {
        PRINT_DEBUG(DEBUG, "Did not get answer in time. Consider it as failure");
    }
    return ( status == std::future_status::ready && loginStatus.get() );
}

bool AMQPConnection::_declareExchanges() const
{
    for ( const auto & exchangeName : _exchangesName )
        if ( declareExchange( exchangeName, MAX_WAIT_TIME_FOR_ANSWER_IN_SEC ) == false )
            return false;
    return true;
}

bool AMQPConnection::_declareQueue() const
{
    // TODO: change to exclusive (_queueName, false, true, false)
    std::future< bool > declareQueueResult = _connectionHandler.declareQueue(
            _queueName, 
            false, 
            false,
            true );
    std::future_status status = declareQueueResult.wait_for(std::chrono::seconds( MAX_WAIT_TIME_FOR_ANSWER_IN_SEC ));
    if( status != std::future_status::ready )
    {
        PRINT_DEBUG(DEBUG, "Did not get answer in time. Consider it as failure");
    }
    return ( status == std::future_status::ready && declareQueueResult.get() );
}

bool AMQPConnection::_removeQueue() const
{
    std::future< bool > removeQueueResult = _connectionHandler.removeQueue(
            _queueName );
    std::future_status status = removeQueueResult.wait_for(std::chrono::seconds( MAX_WAIT_TIME_FOR_ANSWER_IN_SEC ));
    if( status != std::future_status::ready )
    {
        PRINT_DEBUG(DEBUG, "Did not get answer in time. Consider it as failure");
    }
    return ( status == std::future_status::ready && removeQueueResult.get() );
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
    _connectionHandler.publish( exchangeName, routingKey, message );
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
        AMQPConnection::_bind( _exchangesName[ 0 ], _queueName, routingKey );
    return ReturnStatus::Ok;
}

bool AMQPConnection::connected() const
{
    return _isConnected;
}
