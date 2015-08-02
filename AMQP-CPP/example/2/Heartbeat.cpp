#include "Heartbeat.h"
#include "AMQPConnectionHandler.h"

#define AdminExchangeName "admin"
#define AdminQueueName "admin"
#define AdminRoutingKey "admin"
namespace AMQP {
Heartbeat::Heartbeat( AMQPConnectionHandler * connectionHandler ):
    _connectionHandler( connectionHandler )
{}

void Heartbeat::initialize()
{
    //TODO:
    //Shoud go to the place bad code goes
    std::shared_ptr< std::promise< bool > > deferedResultSetter = std::make_shared< std::promise< bool > > ();
    _connectionHandler->declareExchange(AdminExchangeName, fanout, false, deferedResultSetter );

    auto & queueHndl = _connectionHandler->_channel->declareQueue( AdminQueueName, 0);
    queueHndl.onSuccess([ this ]() { 
            std::cout <<"Admin queue declared OK \n";
            _connectionHandler->_channel->consume( "admin" ).onReceived([ this ](const AMQP::Message &message,
                    uint64_t deliveryTag, 
                    bool redelivered ) {
                _connectionHandler->_channel->ack( deliveryTag );
                }); 
            _initialized = true;
            } );
    queueHndl.onError( [ this ] ( const char* message ) {
            std::cout <<"Failed declaring admin. error: " << message << std::endl;
            _initialized = false;
            } );

    _connectionHandler->_channel->bindQueue( AdminExchangeName, AdminQueueName, AdminRoutingKey );
}

bool Heartbeat::send( )
{
    //TODO: remove this return true. it simply disables the heartbeat
    return true;

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
        _connectionHandler->_channel->publish( AdminExchangeName, AdminQueueName, AdminRoutingKey );
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
