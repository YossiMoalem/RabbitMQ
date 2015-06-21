#include "AMQPEventLoop.h"
#include "BlockingQueue.h"
#include "AMQPConnectionHandler.h"
#include "RabbitOperation.h"
#include <algorithm>
#include <amqpcpp.h>

namespace AMQP {

AMQPEventLoop::AMQPEventLoop( BlockingQueue<RabbitMessageBase * >  * jobQueue,
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
    int maxFD =  std::max( queueEventFd, 
            std::max( brokerWriteFD, brokerReadFD ) ) +1;

    timeval heartbeatIdenInterval;
    _resetTimeout( heartbeatIdenInterval );

    while( ! _stop )
    {
        FD_ZERO( & readFdSet );
        FD_ZERO( & writeFdSet );
        FD_SET( brokerReadFD, & readFdSet );
        if( _connectionHandler->canHandle() )
        {
            FD_SET( queueEventFd, & readFdSet );
        }
        if( _connectionHandler->pendingSend() )
        {
            FD_SET( brokerWriteFD, & writeFdSet );
        }

        int res = select( maxFD, & readFdSet, & writeFdSet, NULL, &heartbeatIdenInterval);
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
