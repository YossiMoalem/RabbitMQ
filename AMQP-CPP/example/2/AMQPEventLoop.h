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
   AMQPEventLoop( BlockingQueue< RabbitMessageBase * > * jobQueue, 
           AMQPConnectionHandler *                 connectionHandler) ;
   int start();
   void stop();

 private:
   void _handleQueue( );
   void _handleInput( );
   void _handleOutput( );
   void _resetTimeout( timeval & timeoutTimeval );

 private:
   volatile bool                            _stop = false;
   AMQPConnectionHandler *                  _connectionHandler;
   BlockingQueue<RabbitMessageBase * > *    _jobQueue;
};

} //namespace AMQP

#endif
