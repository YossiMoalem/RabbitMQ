#ifndef AMQP_EVENT_LOOP
#define AMQP_EVENT_LOOP

#include "RabbitJobQueue.h"

namespace AMQP {

class RabbitJobHandler;

class Message;

class RabbitEventLoop
{
 public:
   RabbitEventLoop( RabbitJobQueue &  jobQueue, 
           RabbitJobHandler *   jobHHandler );

   int start( int  brokerReadFD,
           int  brokerWriteFD );
   void stop();

 private:
   void _handleQueue( );
   void _handleInput( );
   void _handleOutput( );
   void _resetTimeout( timeval & timeoutTimeval );

 private:
   volatile bool                            _stop = false;
   RabbitJobHandler  *                      _jobHandler;
   RabbitJobQueue &                         _jobQueue;
};

} //namespace AMQP

#endif
