#include "AMQPEventLoop.h"
#include "BlockingQueue.h"
#include "RabbitOperation.h"
#include "AMQPConnectionHandler.h"

#include <amqpcpp.h>

namespace AMQP {

AMQPEventLoop::AMQPEventLoop(  std::function<int( const AMQP::Message& )> onMsgReceivedCB,
        BlockingQueue<RabbitMessageBase * >  * jobQueue ) :
    _connectionHandlers( new AMQPConnectionHandler ( onMsgReceivedCB ) ),
    _jobQueue( jobQueue )
{ }

int AMQPEventLoop::start()
{
    _connectionHandlers->waitForConnection();

    fd_set readFd;
    int queueEventFd = _jobQueue->getFD();
    int brokerReadFD =  _connectionHandlers->getReadFD();
    int maxReadFd = ( queueEventFd > brokerReadFD ) ? queueEventFd + 1 : brokerReadFD + 1 ;


    while( ! _stop )
    {
        timeval heartbeatIdenInterval;
        heartbeatIdenInterval.tv_sec = 5;
        heartbeatIdenInterval.tv_usec = 0;

        FD_ZERO( & readFd );
        FD_SET ( queueEventFd, & readFd );
        FD_SET ( brokerReadFD, & readFd );

        int res = select( maxReadFd, & readFd, NULL, NULL, &heartbeatIdenInterval);
        //TODO: change to res > 0. this is a temp workaround till we will handle the send as described bellow.
        if( res >= 0 )
        {
            if( FD_ISSET( brokerReadFD, & readFd ) )
            {
                _connectionHandlers->handleInput();
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
            //
            // BUG ALLERT: as it is now - if we did not finish the send, nothing will trigger us to do so!
            if ( _connectionHandlers->pendingSend() )
            {
                _connectionHandlers->handleOutput();
            }
        }
        else
        {
            std::cout <<"select returned : " <<res <<" Errno = " <<errno <<std::endl;
        }

        if ( res == 0 ){
//            std::cout <<"should send heartbeat" <<std::endl;
            //TODO: send heartbeat. 
            //If we already sent heartbeat in the last timeout and did not get response - we are in trouble.
        }

        if ( _connectionHandlers->stopEventLoop() )
        {
            _connectionHandlers->setStopEventLoop( false );
            return 1;
        }
    }
    return 0;
}

void AMQPEventLoop::handleQueue( )
{
    RabbitMessageBase * msg = nullptr;
    if( _jobQueue->try_pop( msg ) )
    {
        msg->handle( this );
    }
    delete msg;
}

void AMQPEventLoop::stop( bool terminateNow )
{
    _jobQueue->close();
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
