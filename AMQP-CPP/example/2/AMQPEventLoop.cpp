#include "AMQPEventLoop.h"
#include "BlockingQueue.h"
#include "RabbitOperation.h"
#include "AMQPConnectionHandler.h"
#include <algorithm>
#include <amqpcpp.h>

namespace AMQP {

AMQPEventLoop::AMQPEventLoop(  std::function<int( const AMQP::Message& )> onMsgReceivedCB,
        BlockingQueue<RabbitMessageBase * >  * jobQueue ) :
    _connectionHandlers( new AMQPConnectionHandler ( onMsgReceivedCB, this ) ),
    _jobQueue( jobQueue )
{ }

int AMQPEventLoop::start()
{
    //TODO: should we update stop here or only after we unleash the EL???
    //(It's been a long day, vant think right now....
    _stop = false;
    std::cout << "started event loop" << std::endl;
    _connectionHandlers->waitForConnection();
    std::cout <<"Eventloop unleashed! "<<std::endl;

    fd_set readFd;
    int outgoingMessagesEventFd = _connectionHandlers->getOutgoingMessagesFD();
    int queueEventFd = _jobQueue->getFD();
    int brokerReadFD = _connectionHandlers->getReadFD();
    int maxReadFd =  std::max( outgoingMessagesEventFd, std::max( queueEventFd, brokerReadFD ) ) +1;

    timeval heartbeatIdenInterval;
    heartbeatIdenInterval.tv_sec = 15;
    heartbeatIdenInterval.tv_usec = 50;

    bool lastCallWasTimeOut = false;

    _jobQueue->flush();
    while( ! _stop )
    {
        FD_ZERO( & readFd );
        FD_SET ( outgoingMessagesEventFd, & readFd );
        FD_SET ( queueEventFd, & readFd );
        FD_SET ( brokerReadFD, & readFd );

        int res = select( maxReadFd, & readFd, NULL, NULL, &heartbeatIdenInterval);
        //        std::cout << "select res: " <<res <<std::endl;
        //TODO: change to res > 0. this is a temp workaround till we will handle the send as described bellow.
        if( res > 0 )
        {
            if( FD_ISSET( brokerReadFD, & readFd ) )
            {
                try
                {
                    _connectionHandlers->handleInput();
                }

                catch(...)
                {
                    std::cout << "read failed. closing event loop" <<std::endl;
                    _stop = true;
                }
                lastCallWasTimeOut = false;
                heartbeatIdenInterval.tv_sec = 7;
                heartbeatIdenInterval.tv_usec = 50;
            }
            if( FD_ISSET( queueEventFd, & readFd ) )
            {
                handleQueue();
            }

            //TODO:
            // if we have messages to send, do not try to immediatly send it:
            // 1. register teh write socket with the select.
            // 2. when it is called - send
            // 2.1 after sending, if not everything sent - back to 1. 
            // 2.2 otherwise - do not register write
            if( FD_ISSET( outgoingMessagesEventFd, & readFd ) )
            {
                assert ( _connectionHandlers->pendingSend() );
                try
                {
                    _connectionHandlers->handleOutput();
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
            //std::cout <<" select timeout \n";
            assert (! _connectionHandlers->pendingSend() );
            if (  lastCallWasTimeOut )
            {
                std::cout <<"Timeout expired. disconnect!" <<std::endl;
                _connectionHandlers->closeSocket();
                return 2;
            } else {
                if( _connectionHandlers->handleTimeout() )
                {
                    lastCallWasTimeOut = true;
                }
                heartbeatIdenInterval.tv_sec = 5;
                heartbeatIdenInterval.tv_usec = 50;
            } 
        }
        else
        {
            std::cout << "select returned : " << res << "Errno = " << errno << std::endl;
        }
    }
    std::cout <<"EventLoop stoped 0 "<< std::endl;
    _connectionHandlers->closeSocket();
    return 0;
}

void AMQPEventLoop::handleQueue( )
{
    RabbitMessageBase * msg = nullptr;
     _jobQueue->pop( msg ) ;
     msg->handle( this );
    delete msg;
}

void AMQPEventLoop::stop( bool terminateNow )
{
    if( terminateNow )
    {
        _stop = true;
    } else {
        StopMessage * stopMessage = new StopMessage( true );
        _jobQueue->push( stopMessage );
    }
}

void AMQPEventLoop::publish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message, 
        RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const
{
    _connectionHandlers->doPublish(exchangeName,
            routingKey,
            message,
            operationSucceeded);
}

void AMQPEventLoop::bindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey,  
        RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const
{
    _connectionHandlers->doBindQueue(exchangeName,
            queueName,
            routingKey,
            operationSucceeded);
}

void AMQPEventLoop::unBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const
{
    _connectionHandlers->doUnBindQueue( exchangeName,
            queueName,
            routingKey,
            operationSucceeded );
}

}//namespace AMQP
