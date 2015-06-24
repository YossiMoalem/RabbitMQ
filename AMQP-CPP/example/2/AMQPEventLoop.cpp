#include "AMQPEventLoop.h"
#include "BlockingQueue.h"
#include "RabbitJobManager.h"
#include "RabbitOperation.h"
#include <algorithm>
#include <amqpcpp.h>

namespace AMQP {

AMQPEventLoop::AMQPEventLoop( BlockingQueue<RabbitMessageBase * >  * jobQueue,
        RabbitJobManager * handler ) :
    _handler( handler),
    _jobQueue( jobQueue )
{ }

int AMQPEventLoop::start( int queueEventFD,
        int  brokerReadFD ,
        int  brokerWriteFD )
{
    _stop = false;
    std::cout <<"Eventloop unleashed! "<<std::endl;

    fd_set readFdSet;
    fd_set writeFdSet;
    int maxFD =  std::max( queueEventFD, 
            std::max( brokerWriteFD, brokerReadFD ) ) + 1;

    timeval heartbeatIdenInterval;
    heartbeatIdenInterval.tv_sec = 15;
    heartbeatIdenInterval.tv_usec = 0;

    while( ! _stop )
    {
        FD_ZERO( & readFdSet );
        FD_ZERO( & writeFdSet );
        FD_SET( brokerReadFD, & readFdSet );
        if( _handler->canHandleMessage() )
        {
            FD_SET( queueEventFD, & readFdSet );
        }
        if( _handler->pendingSend() )
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
            if( FD_ISSET( queueEventFD, & readFdSet ) )
            {
                _handleQueue();
            }

            if( FD_ISSET( brokerWriteFD, & writeFdSet ) )
            {
                _handleOutput();
            }
        }
        else if ( res == 0 ){
            _handler->handleTimeout();
            _resetTimeout( heartbeatIdenInterval );
        }
        else
        {
            std::cout << "select returned : " << res << "Errno = " << errno << std::endl;
        }
    }
    std::cout <<"EventLoop stoped 0 "<< std::endl;
    _handler->stopEventLoop( true, dummyResultSetter );
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
    while ( _handler->canHandleMessage() && _jobQueue->try_pop( msg ) )
    {
        msg->handle( );
        delete msg;
    }
}

void AMQPEventLoop::_handleOutput()
{
    try
    {
        _handler->handleOutput();
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
        _handler->handleInput();
    }

    catch(...)
    {
        std::cout << "read failed. closing event loop" <<std::endl;
        stop();
    }
}

}//namespace AMQP
