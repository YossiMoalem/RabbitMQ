#include "AMQPEventLoop.h"
#include "BlockingQueue.h"
#include "RabbitMessage.h"
#include "AMQPConnectionHandler.h"

#include <amqpcpp.h>

namespace AMQP {

AMQPEventLoop::AMQPEventLoop(  std::function<int( const AMQP::Message& )> onMsgReceivedCB,
        BlockingQueue<RabbitMessageBase * >  * jobQueue ) :
    _connectionHandlers( new AMQPConnectionHandler( onMsgReceivedCB ) ),
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

    while( true )
    {
        FD_ZERO( & readFd );
        FD_SET ( queueEventFd, & readFd );
        FD_SET ( brokerReadFD, & readFd );

        select( maxReadFd, & readFd, NULL, NULL, NULL );
        if( FD_ISSET( brokerReadFD, & readFd ) )
        {
            _connectionHandlers->handleInput();
        }
        if( FD_ISSET( queueEventFd, & readFd ) )
        {
            handleQueue();
        }

        if ( _connectionHandlers->pendingSend() )
        {
            _connectionHandlers->handleOutput();
        }
    }
    return 0;
}


void AMQPEventLoop::handleQueue( )
{
    RabbitMessageBase * msg = nullptr;
    if( _jobQueue->try_pop( msg ) )
    {
        switch( msg->messageType() )
        {
            case MessageType::Post:
                {
                    PostMessage * postMessage = static_cast< PostMessage* >( msg );
                    _connectionHandlers->doPublish( postMessage->exchangeName(), 
                            postMessage->routingKey(), 
                            postMessage->message(), 
                            postMessage->resultSetter() );
                    delete msg;
                }
                break;
            case MessageType::Bind:
                {
                    BindMessage * bindMessage = static_cast< BindMessage* >( msg );
                    _connectionHandlers->doBindQueue(bindMessage->exchangeName(), 
                            bindMessage->queueName(), 
                            bindMessage->routingKey(), 
                            bindMessage->resultSetter() ); 
                }
                break;
            case MessageType::UnBind:
                {
                    UnBindMessage * unBindMessage = static_cast< UnBindMessage* >( msg );
                    _connectionHandlers->doUnBindQueue(unBindMessage->exchangeName(), 
                            unBindMessage->queueName(), 
                            unBindMessage->routingKey(), 
                            unBindMessage->resultSetter() ); 
                }
                break;
        }
    }
}


}//namespace AMQP
