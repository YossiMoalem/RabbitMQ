#include "RabbitJobManager.h"
#include "AMQPConnectionDetails.h"
#include "AMQPEventLoop.h"

namespace AMQP {

RabbitJobManager::RabbitJobManager( std::function<int( const AMQP::Message& )> onMsgReceivedCB ) :
    _connectionState( [ this] () { terminate( ); } ),
    _connectionHandler( new AMQPConnectionHandler( onMsgReceivedCB, _connectionState ) ),
    _heartbeat( _connectionHandler )
    {}

RabbitJobManager::~RabbitJobManager( )
{
    delete _connectionHandler;
}

DeferedResult RabbitJobManager::addJob ( RabbitMessageBase * job )
{
    job->setHandler( this );
    auto result = job->deferedResult();
    _jobQueue.push( job );
    return result;
}
bool RabbitJobManager::connect(const AMQPConnectionDetails & connectionParams )
{
    bool connected =_connectionHandler->connect( connectionParams );
    return connected;
}

void RabbitJobManager::startEventLoop()
{
    _eventLoop = new AMQPEventLoop( &_jobQueue,
            this,
            _jobQueue.getFD(),
            _connectionHandler->getReadFD(),
            _connectionHandler->getWriteFD() );
    _eventLoop->start();
}

void RabbitJobManager::stopEventLoop( bool immediate,
    DeferedResultSetter returnValueSetter )
{
    if( immediate )
    {
        _connectionState.disconnecting( returnValueSetter );
        _connectionState.disconnected();
    } else {
        _connectionState.disconnecting( returnValueSetter );
        StopMessage * stopMessage = new StopMessage( true );
        addJob( stopMessage );
    }
}

bool RabbitJobManager::canHandleMessage() const
{
    return _connectionHandler->outgoingBufferSize() < _outgoingBufferHighWatermark;
}

void RabbitJobManager::handleTimeout()
{
    if( _heartbeat.send() == false )
    {
        stopEventLoop( true, dummyResultSetter );
    }
}

void RabbitJobManager::terminate()
{
    _heartbeat.invalidate();
    _eventLoop->stop();
    _connectionHandler->closeSocket();
}

} //namespace AMQP
