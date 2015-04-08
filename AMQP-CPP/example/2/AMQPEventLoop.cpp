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
    //    if (!_connected )
    //        return 1;

    fd_set readFd;
    int queueEventFd = _jobQueue->getFD();
    int brokerReadFD =  _connectionHandlers->getReadFD();
    int maxReadFd = ( queueEventFd > brokerReadFD ) ? queueEventFd + 1 : brokerReadFD + 1 ;

    while( ! _stop )
    {
        FD_ZERO( & readFd );
        FD_SET ( queueEventFd, & readFd );
        FD_SET ( brokerReadFD, & readFd );

        int res = select( maxReadFd, & readFd, NULL, NULL, NULL );
        if( res > 0 )
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
            if ( _connectionHandlers->pendingSend() )
            {
                _connectionHandlers->handleOutput();
            }
        } else {
            std::cout <<"select returned : " <<res <<" Errno = " <<errno <<std::endl;
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
