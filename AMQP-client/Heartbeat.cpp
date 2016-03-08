#include "Heartbeat.h"
#include "RabbitConnection.h"
#include "Debug.h"

#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace AMQP {
Heartbeat::Heartbeat( const RabbitConnection & dataConnection ):
    _connection( [] (const AMQP::Message&) { return 0;},  dataConnection ),
    _heartbeatSent( false ),
    _initialized( false )
{
    _adminQueueName = "admin_" + std::to_string(getpid()) + "_" + std::to_string( time( nullptr ) ) ;
}

void Heartbeat::_createAdminQueue()
{
    PRINT_DEBUG(DEBUG, "Initializing Heartbeat");
    _connection.declareExchange(_adminExchangeName, fanout, false, 
            std::make_shared< std::promise< bool > > () );
    _connection.declareQueue( _adminQueueName, false, false, true, 
            std::make_shared< std::promise< bool > > () ); 
    _connection.doBindQueue( _adminExchangeName, _adminQueueName, _adminRoutingKey, 
            std::make_shared< std::promise< bool > > () );
    PRINT_DEBUG(DEBUG, "Heartbeat Initialized ");
}

void Heartbeat::_initialize()
{
    _createAdminQueue();
    _initialized = true;
    PRINT_DEBUG(DEBUG, "Trying to initialized Heartbeat");
}

void Heartbeat::_sendHeartbeat()
{
    DeferredResultSetter deferedResultSetter = std::make_shared< std::promise< bool > > ();
    _connection.doPublish( _adminExchangeName, _adminRoutingKey, _adminQueueName, deferedResultSetter );
    _heartbeatSent = true;
    PRINT_DEBUG(DEBUG, "Sending HB message");
}

bool Heartbeat::send( )
{
    PRINT_DEBUG(DEBUG, "Checking Heartbeat");
    if ( ! _initialized )
    {
        _initialize();
    } else if ( ! _heartbeatSent )
    {
        _sendHeartbeat();
    } else {
        PRINT_DEBUG(DEBUG, "Did not get HB response, probably disconnected");
        return false;
    }
    return true;
}
void Heartbeat::invalidate()
{
    _heartbeatSent = false;
    _initialized = false;
}

void Heartbeat::reset()
{
    _heartbeatSent = false;
}

} //namespace AMQP
