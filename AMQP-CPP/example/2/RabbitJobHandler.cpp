#include "RabbitJobHandler.h"
#include "RabbitEventLoop.h"
#include "RabbitOperation.h"
#include "Types.h"

namespace AMQP {

RabbitJobHandler::RabbitJobHandler( OnMessageReveivedCB onMsgReceivedCB, RabbitJobQueue & jobQueue ) :
    _connectionState( [ this ] () { 
        _heartbeat.invalidate();
        _eventLoop->stop();
        _connection->closeSocket();
        } ), 
    _connection( new RabbitConnection( onMsgReceivedCB, _connectionState ) ),
    _heartbeat( *_connection ),
    _jobQueue( jobQueue ),
    _eventLoop( new RabbitEventLoop( _jobQueue, this ) )
    {}

RabbitJobHandler::~RabbitJobHandler( )
{
    delete _connection;
    delete _eventLoop;
}

bool RabbitJobHandler::start( const RabbitConnectionDetails & connectionParams )
{
    DeferedResultSetter connectedReturnValueSetter( new std::promise< bool > );
    auto connectedReturnValue = connectedReturnValueSetter->get_future();
    _eventLoopThread = std::thread( std::bind( &RabbitJobHandler::doStart, this, 
                connectionParams, connectedReturnValueSetter ) );
    connectedReturnValue.wait();
    _jobQueue.clear();
    bool connected = connectedReturnValue.get();
    if ( !connected )
    {
        _eventLoopThread.detach();
    }
    connectedReturnValueSetter.reset();
    return connected;
}

void RabbitJobHandler::doStart( const RabbitConnectionDetails & connectionParams, 
        DeferedResultSetter connectedReturnValueSetter )
{
    bool connected = _connection->connect( connectionParams );
    connectedReturnValueSetter->set_value( connected );
    if( connected )
    {
        _eventLoop->start(
                _connection->readFD(),
                _connection->writeFD() );
    }
}

void RabbitJobHandler::stopEventLoop( bool immediate,
    DeferedResultSetter returnValueSetter )
{
    if( immediate )
    {
        if( _connectionState.disconnecting( returnValueSetter ) )
        {
            _connectionState.disconnected();
        }
    } else {
        if( _connectionState.disconnecting( returnValueSetter ) )
        {
            StopMessage * stopMessage = new StopMessage( true );
            _jobQueue.addJob( stopMessage );
        }
    }
}

bool RabbitJobHandler::canHandleMessage() const
{
    return _connection->outgoingBufferSize() < _outgoingBufferHighWatermark; 
}

void RabbitJobHandler::handleTimeout()
{
    if( _heartbeat.send() == false )
    {
        stopEventLoop( true, dummyResultSetter );
    }
}

void RabbitJobHandler::waitForDisconnection()
{
    _eventLoopThread.join();
}
} //namespace AMQP
