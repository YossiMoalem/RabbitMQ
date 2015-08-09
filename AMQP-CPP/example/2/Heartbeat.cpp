#include "Debug.h"
#include "Heartbeat.h"
#include "AMQPConnectionHandler.h"

//#include <sys/types.h>
//#include <unistd.h>
//#include <iostream>
//#include <string.h>
//#include <sstream>
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>



#define AdminExchangeName "admin"
#define AdminRoutingKey "admin"
namespace AMQP {
Heartbeat::Heartbeat( AMQPConnectionHandler * connectionHandler ):
    _connectionHandler( connectionHandler )
{
    //TODO: add to AdminQueueName the hostname and maybe other param as well and not just the hostname (hubname maybe?)
    /* initialize random seed: */
    srand (time(NULL));
    /* generate secret number between 1 and 100000 : */
    int iSecret = rand() % 100000 + 1;
    AdminQueueName = "admin" + std::to_string(getpid()) + std::to_string(iSecret) ;

}


void Heartbeat::initialize()
{
    //TODO:
    //Should go to the place bad code goes


    std::shared_ptr< std::promise< bool > > deferedResultSetter = std::make_shared< std::promise< bool > > ();
    _connectionHandler->declareExchange(AdminExchangeName, fanout, false, deferedResultSetter );

    int flags = 0;
//    flags |= AMQP::durable;
//    flags |= AMQP::exclusive;
    flags |= AMQP::autodelete;

    AMQP::Table arguments;
//    arguments["x-dead-letter-exchange"] = "some-exchange";
    arguments["x-message-ttl"] = 30 * 1000; //time in ms before message is discarded
//    arguments["x-expires"] = 10 * 1000; //time in ms before queue is automatically deleted if idle

    auto & queueHndl = _connectionHandler->_channel->declareQueue( AdminQueueName, flags, arguments );
    queueHndl.onSuccess([ this ]() { 
            PRINT_DEBUG(DEBUG, "Admin queue declared OK");
            _connectionHandler->_channel->consume( AdminQueueName ).onReceived([ this ](const AMQP::Message &message,
                    uint64_t deliveryTag, 
                    bool redelivered ) {
                _connectionHandler->_channel->ack( deliveryTag );
                }); 
            _initialized = true;
            } );
    queueHndl.onError( [ this ] ( const char* message ) {
            PRINT_DEBUG(DEBUG, "Failed declaring admin. error: " << message );
            _initialized = false;
            } );

    _connectionHandler->_channel->bindQueue( AdminExchangeName, AdminQueueName, AdminRoutingKey );
}

bool Heartbeat::send( )
{
    if ( ! _initialized )
    {
        if( ! _initializeCalled )
        {
            _initializeCalled = true;
            initialize();
            return true;
        } else {
            return false;
        }
    }
    if ( ! _heartbeatSent )
    {
        _connectionHandler->_channel->publish( AdminExchangeName, AdminRoutingKey, AdminQueueName );
        _heartbeatSent = true;
        return true;
    } 
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
    _heartbeatSent = false;
}


} //namespace AMQP
