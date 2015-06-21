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
           RabbitJobManager *   connectionHandler,
           int                  queueEventFD,
           int                  brokerReadFD,
           int                  brokerWriteFD );
   int start();
   void stop();

 private:
   void _handleQueue( );
   void _handleInput( );
   void _handleOutput( );
   void _resetTimeout( timeval & timeoutTimeval );

 private:
   volatile bool                            _stop = false;
   RabbitJobManager *                       _handler;
   BlockingQueue<RabbitMessageBase * > *    _jobQueue;
   int                                      _queueEventFD;
   int                                      _brokerReadFD ;
   int                                      _brokerWriteFD;
};

} //namespace AMQP

#endif
