#ifndef AMQP_EVENT_LOOP
#define AMQP_EVENT_LOOP

#include <functional>
#include <memory>

namespace AMQP {

template < typename T >
    class BlockingQueue;
class RabbitMessageBase;
class RabbitJobManager;

class Message;

class AMQPEventLoop
{
 public:
   AMQPEventLoop( BlockingQueue< RabbitMessageBase * > * jobQueue, 
           RabbitJobManager *   connectionHandler );

   int start( int queueEventFD,
           int  brokerReadFD,
           int  brokerWriteFD );
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
};

} //namespace AMQP

#endif
