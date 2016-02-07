#include "RabbitEventLoop.h"
#include "RabbitJobHandler.h"
#include "RabbitOperation.h"
#include "Debug.h"
#include <algorithm>
#include <amqpcpp.h>

namespace AMQP {

RabbitEventLoop::RabbitEventLoop( RabbitJobQueue & jobQueue,
        RabbitJobHandler* handler ) :
    _jobHandler( handler),
    _jobQueue( jobQueue )
{ }

int RabbitEventLoop::start( int  brokerReadFD ,
        int  brokerWriteFD )
{
    _stop = false;
    PRINT_DEBUG(DEBUG, "Eventloop unleashed! ");

    int queueEventFD = _jobQueue.getFD();

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
        if( _jobHandler->canHandleMessage() )
        {
            FD_SET( queueEventFD, & readFdSet );
        }
        if( _jobHandler->pendingSend() )
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
            _jobHandler->handleTimeout();
            _resetTimeout( heartbeatIdenInterval );
        }
        else
        {
            PRINT_DEBUG(DEBUG,  "select returned : " << res << "Errno = " << errno);
        }
    }
    PRINT_DEBUG(DEBUG, "EventLoop stopped 0 ");
    _jobHandler->stopEventLoop( dummyResultSetter );
    return 0;
}

void RabbitEventLoop::stop()
{
    _stop = true;
}

void RabbitEventLoop::_resetTimeout( timeval & timeoutTimeval )
{
    timeoutTimeval.tv_sec = 7;
    timeoutTimeval.tv_usec = 0;
}

void RabbitEventLoop::_handleQueue( )
{
    RabbitMessageBase * msg = nullptr;
    while ( _jobHandler->canHandleMessage() && _jobQueue.tryPop( msg ) )
    {
        msg->handle( );
        delete msg;
    }
}

void RabbitEventLoop::_handleOutput()
{
    try
    {
        _jobHandler->handleOutput();
    }
    catch( const std::exception &  e )
    {
        PRINT_DEBUG( DEBUG,  "send failedclosing event loop. exception: "<< e.what() );
        stop();
    }
}

void RabbitEventLoop::_handleInput()
{
    try
    {
        _jobHandler->handleInput();
    }

    catch( const std::exception & e )
    {
        PRINT_DEBUG( DEBUG,  "read failed. closing event loop. exception: "<< e.what() );
        stop();
    }
}

}//namespace AMQP
