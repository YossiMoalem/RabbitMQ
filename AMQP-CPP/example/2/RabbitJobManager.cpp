#include "RabbitJobManager.h"
#include "AMQPConnectionDetails.h"
#include "AMQPEventLoop.h"
#include "RabbitOperation.h"

namespace AMQP {

RabbitJobManager::RabbitJobManager( std::function<int( const AMQP::Message& )> onMsgReceivedCB ) :
    _connectionState( [ this ] () { terminate( ); } ),
    _connectionHandler( new AMQPConnectionHandler( onMsgReceivedCB, _connectionState ) ),
    _heartbeat( _connectionHandler ),
    _eventLoop( new AMQPEventLoop( &_jobQueue,
            this ) )
    {}

RabbitJobManager::~RabbitJobManager( )
{
    delete _connectionHandler;
    delete _eventLoop;
}

DeferedResult RabbitJobManager::addJob ( RabbitMessageBase * job )
{
    auto result = job->deferedResult();
    job->setHandler( this );
    _jobQueue.push( job );
    return result;
}

bool RabbitJobManager::start( const AMQPConnectionDetails & connectionParams )
{
    DeferedResultSetter connectedReturnValueSetter( new std::promise< bool > );
    auto connectedReturnValue =  connectedReturnValueSetter->get_future();
    _eventLoopThread = std::thread( std::bind( &RabbitJobManager::doStart, this, 
                connectionParams, connectedReturnValueSetter ) );
    connectedReturnValue.wait();
    bool connected = connectedReturnValue.get();
    if ( !connected )
    {
        _eventLoopThread.detach();
    }
    connectedReturnValueSetter.reset();
    return connected;
}

void RabbitJobManager::doStart( const AMQPConnectionDetails & connectionParams, 
        DeferedResultSetter connectedReturnValueSetter )
{
    bool connected =_connectionHandler->connect( connectionParams );
    connectedReturnValueSetter->set_value( connected );
    if( connected )
    {
        _eventLoop->start(
                _jobQueue.getFD(),
                _connectionHandler->getReadFD(),
                _connectionHandler->getWriteFD() );
    }
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


void RabbitJobManager::waitForDisconnection()
{
    _eventLoopThread.join();
}
} //namespace AMQP
