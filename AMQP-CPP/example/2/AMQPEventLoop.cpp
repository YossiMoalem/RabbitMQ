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
    _stop = false;
    std::cout <<"Eventloop unleashed! "<<std::endl;

    fd_set readFdSet;
    fd_set writeFdSet;
    int outgoingMessagesEventFd = _connectionHandlers->getOutgoingMessagesFD();
    int queueEventFd = _jobQueue->getFD();
    int brokerWriteFD = _connectionHandlers->getWriteFD();
    int brokerReadFD = _connectionHandlers->getReadFD();
    int maxReadFd =  std::max( std::max( outgoingMessagesEventFd, queueEventFd ), 
            std::max( brokerWriteFD, brokerReadFD ) ) +1;

    timeval heartbeatIdenInterval;
    _resetTimeout( heartbeatIdenInterval );

    bool lastCallWasTimeOut = false;

    _jobQueue->flush();
    while( ! _stop )
    {
        FD_ZERO( & readFdSet );
        FD_SET ( outgoingMessagesEventFd, & readFdSet );
        FD_SET ( queueEventFd, & readFdSet );
        FD_SET ( brokerReadFD, & readFdSet );

        FD_ZERO( & writeFdSet );
        if( _connectionHandlers->pendingSend() )
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
                    _connectionHandlers->handleInput();
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
            assert (! _connectionHandlers->pendingSend() );
            if (  lastCallWasTimeOut )
            {
                std::cout <<"Did not receive anythiong even after sending hertbeat. disconnect!" <<std::endl;
                _connectionHandlers->closeSocket();
                return 2;
            } else {
                if( _connectionHandlers->handleTimeout() )
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
    _connectionHandlers->closeSocket();
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
        RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    _connectionHandlers->doPublish(exchangeName,
            routingKey,
            message,
            operationSucceeded);
}

void AMQPEventLoop::bindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey,  
        RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    _connectionHandlers->doBindQueue(exchangeName,
            queueName,
            routingKey,
            operationSucceeded);
}

void AMQPEventLoop::unBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    _connectionHandlers->doUnBindQueue( exchangeName,
            queueName,
            routingKey,
            operationSucceeded );
}

void AMQPEventLoop::login( const std::string & userName,
           const std::string & password, 
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    _connectionHandlers->login( userName, password, operationSucceeded );
}

void AMQPEventLoop::declareExchange( const std::string & exchangeName, 
           ExchangeType exchangetype,
           bool durable,
           RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    _connectionHandlers-> declareExchange( exchangeName, exchangetype, durable, operationSucceeded );
}

void AMQPEventLoop::declareQueue( const std::string & queueName, 
            bool durable, 
            bool exclusive, 
            bool autoDelete,
            RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    _connectionHandlers-> declareQueue( queueName, durable, exclusive, autoDelete, operationSucceeded );
}
}//namespace AMQP
