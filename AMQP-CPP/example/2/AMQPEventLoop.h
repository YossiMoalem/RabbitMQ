#ifndef AMQP_EVENT_LOOP
#define AMQP_EVENT_LOOP

#include <functional>
#include <memory>

#include "RabbitOperation.h"

namespace AMQP {

template < typename T >
    class BlockingQueue;
class RabbitMessageBase;

class Message;

class AMQPEventLoop
{
 public:
   AMQPEventLoop(  std::function<int( const AMQP::Message& )> onMsgReceivedCB, 
           BlockingQueue< RabbitMessageBase * > * jobQueue, 
           AMQPConnectionHandler *                 connectionHandler) ;
   int start();
   void stop();

   //TODO: this is for the connection handler to be able to break from login wait
   //in case the event loop exits. Needs to be removed when login moved to event loop thread
   bool active() { return ! _stop ; } 

 private:
   void handleQueue( );
   void _resetTimeout( timeval & timeoutTimeval );
 private:
   volatile bool                                     _stop = false;
    AMQPConnectionHandler *                 _connectionHandler;
   BlockingQueue<RabbitMessageBase * > *    _jobQueue;
};

} //namespace AMQP

#endif
