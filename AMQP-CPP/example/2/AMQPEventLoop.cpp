#include "AMQPEventLoop.h"
#include "BlockingQueue.h"
#include "AMQPConnectionHandler.h"
#include "RabbitOperation.h"
#include <algorithm>
#include <amqpcpp.h>

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
    int maxReadFd =  std::max( queueEventFd, 
            std::max( brokerWriteFD, brokerReadFD ) ) +1;

    timeval heartbeatIdenInterval;
    _resetTimeout( heartbeatIdenInterval );

    bool lastCallWasTimeOut = false;

    _jobQueue->flush();
    while( ! _stop )
    {
        FD_ZERO( & readFdSet );
        FD_SET ( queueEventFd, & readFdSet );
        FD_SET ( brokerReadFD, & readFdSet );

        FD_ZERO( & writeFdSet );
        if( _connectionHandler->pendingSend() )
        {
            FD_SET ( brokerWriteFD, & writeFdSet );
        }

        int res = select( maxReadFd, & readFdSet, & writeFdSet, NULL, &heartbeatIdenInterval);
        if( res > 0 )
        {
            if( FD_ISSET( brokerReadFD, & readFdSet ) )
            {
                try
                {
                    _connectionHandler->handleInput();
                }

                catch(...)
                {
                    std::cout << "read failed. closing event loop" <<std::endl;
                    _stop = true;
                }
                lastCallWasTimeOut = false;
                _resetTimeout( heartbeatIdenInterval );
            }
            if( FD_ISSET( queueEventFd, & readFdSet ) )
            {
                handleQueue();
            }

            if( FD_ISSET( brokerWriteFD, & writeFdSet ) )
            {
                assert ( _connectionHandler->pendingSend() );
                try
                {
                    _connectionHandler->handleOutput();
                }
                catch(...)
                {
                    std::cout << "send failedclosing event loop" <<std::endl;
                    _stop = true;
                } 
            }
        }
        else if ( res == 0 ){
            //TODO: hide impl.
            assert (! _connectionHandler->pendingSend() );
            if (  lastCallWasTimeOut )
            {
                std::cout <<"Did not receive anythiong even after sending hertbeat. disconnect!" <<std::endl;
                _connectionHandler->closeSocket();
                return 2;
            } else {
                if( _connectionHandler->handleTimeout() )
                {
                    lastCallWasTimeOut = true;
                }
                _resetTimeout( heartbeatIdenInterval );
            } 
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

void AMQPEventLoop::_resetTimeout( timeval & timeoutTimeval )
{
    timeoutTimeval.tv_sec = 7;
    timeoutTimeval.tv_usec = 0;
} 

void AMQPEventLoop::handleQueue( )
{
    RabbitMessageBase * msg = nullptr;
    while ( _connectionHandler->canHandle() && _jobQueue->try_pop( msg ) )
    {
        msg->handle( );
        delete msg;
    }
}

void AMQPEventLoop::stop()
{
    _stop = true;
}

}//namespace AMQP
