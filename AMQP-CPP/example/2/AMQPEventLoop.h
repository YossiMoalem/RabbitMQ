#ifndef AMQP_EVENT_LOOP
#define AMQP_EVENT_LOOP

#include <functional>

namespace AMQP {

template < typename T >
    class BlockingQueue;
class RabbitMessageBase;
class AMQPConnectionHandler;
class Message;

class AMQPEventLoop
{
 public:
   AMQPEventLoop(  std::function<int( const AMQP::Message& )> onMsgReceivedCB, 
           BlockingQueue< RabbitMessageBase * > * jobQueue ) ;
   int start();

   //TODO: this is temporary WA untill I'll move all actions to work via the eventloop.
   //till then, client sends commands to the broker, so it needs the connection handler.
   //this needs to be removed!!!!!
   AMQPConnectionHandler* connectionHandler() { return _connectionHandlers; } 

 private:
   void handleQueue( );

 private:
   AMQPConnectionHandler *               _connectionHandlers;
   BlockingQueue<RabbitMessageBase * > * _jobQueue;
};

} //namespace AMQP

#endif
