#include "RabbitJobManager.h"
#include "AMQPConnectionDetails.h"
#include "AMQPEventLoop.h"

namespace AMQP {

RabbitJobManager::RabbitJobManager( std::function<int( const AMQP::Message& )> onMsgReceivedCB ) :
    _connectionHandler( new AMQPConnectionHandler( onMsgReceivedCB ) ),
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

void RabbitJobManager::stopEventLoop( bool immediate )
{
    RabbitMessageBase::DeferedResultSetter emptySetter;
    stopEventLoop( immediate, emptySetter );
}

void RabbitJobManager::stopEventLoop( bool immediate,
    RabbitMessageBase::DeferedResultSetter returnValueSetter )
{
    if( immediate )
    {
        _eventLoop->stop();
        _connectionHandler->closeSocket();
        if( returnValueSetter )
        {
            returnValueSetter->set_value( true );
        }
        _heartbeat.invalidate();
    } else {
        StopMessage * stopMessage = new StopMessage( true );
        //TODO: Copy the deferedResultSetter to the new message
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
        stopEventLoop( true );
    }
}

} //namespace AMQP
