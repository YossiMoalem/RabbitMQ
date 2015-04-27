#include "Heartbeat.h"
#include "AMQPConnectionHandler.h"

namespace AMQP {
Heartbeat::Heartbeat( AMQPConnectionHandler * connectionHandler ):
    _connectionHandler( connectionHandler )
{}

void Heartbeat::initialize()
{
    //TODO:
    //Shoud go to the place bad code goes
    _connectionHandler->declareExchange("admin", fanout, false);

    auto & queueHndl = _connectionHandler->_channel->declareQueue( "admin", 0);
    queueHndl.onSuccess([ this ]() { 
            std::cout <<"Admin queue declared OK \n";
            _connectionHandler->_channel->consume( "admin" ).onReceived([ this ](const AMQP::Message &message,
                    uint64_t deliveryTag, 
                    bool redelivered ) {
                _connectionHandler->_channel->ack( deliveryTag );
//                std::cout<<" Got: " << message.message() <<" from RK" << message.routingKey() <<std::endl;
                }); 
            _initialized = true;
            } );
    queueHndl.onError( [ this ] ( const char* message ) {
            std::cout <<"Failed declaring admin. error: " << message << std::endl;
            _initialized = false;
            } );

    _connectionHandler->_channel->bindQueue( "admin", "admin", "admin");
}

bool Heartbeat::send( )
{
    if ( _initialized )
    {
//        std::cout <<"TO after we are connected. Sending HB" <<std::endl;
        _connectionHandler->_channel->publish( "admin", "admin", "admin");
    } else {
        initialize() ;
    }
    return true;
}
void Heartbeat::invalidate()
{
    _initialized = false;
}

} //namespace AMQP
