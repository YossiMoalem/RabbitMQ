#include "AMQPEventLoop.h"
#include "BlockingQueue.h"
#include "AMQPConnectionHandler.h"
#include "RabbitOperation.h"
#include <algorithm>
#include <amqpcpp.h>

#define SET_FD_UPDATE_MAX(fd,set,maxFD)   FD_SET(fd, set);\
    maxFD = (maxFD > fd )? maxFD : fd;

namespace AMQP {

AMQPEventLoop::AMQPEventLoop(  std::function<int( const AMQP::Message& )> onMsgReceivedCB,
        BlockingQueue<RabbitMessageBase * >  * jobQueue,
        AMQPConnectionHandler * connectionHandler ) :
    _connectionHandler( connectionHandler),
    _jobQueue( jobQueue )
{ }

int AMQPEventLoop::start()
{
    _stop = false;
    std::cout <<"Eventloop unleashed! "<<std::endl;

    fd_set readFdSet;
    fd_set writeFdSet;
    int queueEventFd = _jobQueue->getFD();
    int brokerWriteFD = _connectionHandler->getWriteFD();
    int brokerReadFD = _connectionHandler->getReadFD();

    timeval heartbeatIdenInterval;
    _resetTimeout( heartbeatIdenInterval );

    while( ! _stop )
    {
        int maxReadFd = 0;
        FD_ZERO( & readFdSet );
        FD_ZERO( & writeFdSet );
        SET_FD_UPDATE_MAX ( brokerReadFD, & readFdSet, maxReadFd );
        if( _connectionHandler->canHandle() )
        {
            SET_FD_UPDATE_MAX( queueEventFd, & readFdSet, maxReadFd );
        }
        if( _connectionHandler->pendingSend() )
        {
            SET_FD_UPDATE_MAX( brokerWriteFD, & writeFdSet, maxReadFd );
        }

        int res = select( maxReadFd, & readFdSet, & writeFdSet, NULL, &heartbeatIdenInterval);
        if( res > 0 )
        {
            if( FD_ISSET( brokerReadFD, & readFdSet ) )
            {
                _handleInput();
                _resetTimeout( heartbeatIdenInterval );
            }
            if( FD_ISSET( queueEventFd, & readFdSet ) )
            {
                _handleQueue();
            }

            if( FD_ISSET( brokerWriteFD, & writeFdSet ) )
            {
                _handleOutput();
            }
        }
        else if ( res == 0 ){
            //TODO: hide impl.
            assert (! _connectionHandler->pendingSend() );
            _connectionHandler->handleTimeout();
            _resetTimeout( heartbeatIdenInterval );
        }
        else
        {
            std::cout << "select returned : " << res << "Errno = " << errno << std::endl;
        }
    }
    std::cout <<"EventLoop stoped 0 "<< std::endl;
    _connectionHandler->closeSocket();
    return 0;
}

void AMQPEventLoop::stop()
{
    _stop = true;
}

void AMQPEventLoop::_resetTimeout( timeval & timeoutTimeval )
{
    timeoutTimeval.tv_sec = 7;
    timeoutTimeval.tv_usec = 0;
} 

void AMQPEventLoop::_handleQueue( )
{
    RabbitMessageBase * msg = nullptr;
    while ( _connectionHandler->canHandle() && _jobQueue->try_pop( msg ) )
    {
        msg->handle( );
        delete msg;
    }
}

void AMQPEventLoop::_handleOutput()
{
    assert ( _connectionHandler->pendingSend() );
    try
    {
        _connectionHandler->handleOutput();
    }
    catch(...)
    {
        std::cout << "send failedclosing event loop" <<std::endl;
        stop();
    } 
}

void AMQPEventLoop::_handleInput()
{
    try
    {
        _connectionHandler->handleInput();
    }

    catch(...)
    {
        std::cout << "read failed. closing event loop" <<std::endl;
        stop();
    }
}

}//namespace AMQP
