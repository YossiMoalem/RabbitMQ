#include "RabbitJobManager.h"
#include "AMQPConnectionDetails.h"
#include "AMQPEventLoop.h"

namespace AMQP {

RabbitJobManager::RabbitJobManager( std::function<int( const AMQP::Message& )> onMsgReceivedCB ) :
    _connectionState( [ this ] () { terminate( ); } ),
    _connectionHandler( new AMQPConnectionHandler( onMsgReceivedCB, _connectionState ) ),
    _heartbeat( _connectionHandler )
    {}

RabbitJobManager::~RabbitJobManager( )
{
    delete _connectionHandler;
}

DeferedResult RabbitJobManager::addJob ( RabbitMessageBase * job )
{
    //This is a sort of way to check if the event loop has started.
    //Need a nicer way. 
    //e.g. from connected to start the EL
    //which also answer my question, who should start the EL, the client or the manager.
    //The answer is, two are fighting, the third gets it.
    auto result = job->deferedResult();
    if( _connectionState.isConnected() )
    {
    job->setHandler( this );
    _jobQueue.push( job );
    } else {
        job->resultSetter()->set_value( false );
    }
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
