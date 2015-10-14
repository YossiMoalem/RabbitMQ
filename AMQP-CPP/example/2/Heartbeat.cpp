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
    _connection( [] (const AMQP::Message&) { return 0;},  dataConnection )
{
    _adminQueueName = "admin_" + std::to_string(getpid()) + "_" + std::to_string( time( nullptr ) ) ;
}

void Heartbeat::initialize()
{
    //TODO: initialize in a more civilized manar...
    PRINT_DEBUG(DEBUG, "Initializing Heartbeat");
    DeferedResultSetter deferedResultSetter1 = std::make_shared< std::promise< bool > > ();
    DeferedResultSetter deferedResultSetter2 = std::make_shared< std::promise< bool > > ();
    DeferedResultSetter deferedResultSetter3 = std::make_shared< std::promise< bool > > ();
    _connection.declareExchange(_adminExchangeName, fanout, false, deferedResultSetter1 );
    _connection.declareQueue( _adminQueueName, false, false, true, deferedResultSetter2 ); 
    _connection.doBindQueue( _adminExchangeName, _adminQueueName, _adminRoutingKey, deferedResultSetter3);
    _initialized = true;
    PRINT_DEBUG(DEBUG, "Heartbeat Initialized ");
}

bool Heartbeat::send( )
{
    PRINT_DEBUG(DEBUG, "Chacking Heartbeat");
    if ( ! _initialized )
    {
        if( ! _initializeCalled )
        {
            initialize();
            _initializeCalled = true;
            PRINT_DEBUG(DEBUG, "Trying to initialized Heartbeat");
            return true;
        } else {
            PRINT_DEBUG(DEBUG, "Called initalize, but we are still not initialized, probably not connected");
            return false;
        }
    }
    if ( ! _heartbeatSent )
    {
        DeferedResultSetter deferedResultSetter = std::make_shared< std::promise< bool > > ();
        _connection.doPublish( _adminExchangeName, _adminRoutingKey, _adminQueueName, deferedResultSetter );
        _heartbeatSent = true;
        PRINT_DEBUG(DEBUG, "Sending HB message");
        return true;
    } 
    PRINT_DEBUG(DEBUG, "Did not get HB response, probably disconnected");
    return false;
}
void Heartbeat::invalidate()
{
    _heartbeatSent = false;
    _initializeCalled = false;
    _initialized = false;
}

void Heartbeat::reset()
{
    PRINT_DEBUG(DEBUG, "Reseting Heartbeat." );
    _heartbeatSent = false;
}

} //namespace AMQP
