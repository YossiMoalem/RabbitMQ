#include "RabbitJobManager.h"
#include "AMQPConnectionDetails.h"
#include "AMQPEventLoop.h"
#include "AMQPConnectionHandler.h"

namespace AMQP {

RabbitJobManager::RabbitJobManager( std::function<int( const AMQP::Message& )> onMsgReceivedCB ) :
    _connectionHandler( new AMQPConnectionHandler( onMsgReceivedCB ) ) 
    {}

RabbitJobManager::~RabbitJobManager( )
{
    delete _connectionHandler;
}

DeferedResult RabbitJobManager::addJob ( RabbitMessageBase * job )
{
    job->setHandler( _connectionHandler );
    auto result = job->deferedResult();
    _jobQueue.push( job );
    return result;
}
bool RabbitJobManager::connect(const AMQPConnectionDetails & connectionParams )
{
    return _connectionHandler->connect( connectionParams );
}

void RabbitJobManager::startEventLoop()
{
    _eventLoop = new AMQPEventLoop( &_jobQueue, _connectionHandler);
    _eventLoop->start();
}

void RabbitJobManager::stopEventLoop( bool immediate )
{
    if( immediate )
    {
        _eventLoop->stop();
    } else {
        //TODO:
        _eventLoop->stop();
    }
}

} //namespace AMQP
