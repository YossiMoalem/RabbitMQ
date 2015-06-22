#ifndef RABBIT_JOB_MANAGER
#define RABBIT_JOB_MANAGER

#include "RabbitOperation.h"
#include "BlockingQueue.h"
#include "AMQPConnectionHandler.h"
#include "Heartbeat.h"
#include "ConnectionState.h"

namespace AMQP {

class AMQPConnectionDetails;
class AMQPEventLoop;

class RabbitJobManager
{
 public:
   RabbitJobManager( std::function<int( const AMQP::Message& )> onMsgReceivedCB ) ;
   ~RabbitJobManager( );

   DeferedResult addJob ( RabbitMessageBase * job );
   void startEventLoop( );
   void stopEventLoop( bool immediate );
   void stopEventLoop( bool immediate,
           DeferedResultSetter returnValueSetter );
   bool connect(const AMQPConnectionDetails & connectionParams );
   bool canHandleMessage() const;
   void handleTimeout();
   bool handleInput()       { return _connectionHandler->handleInput();  }
   bool handleOutput()      { return _connectionHandler->handleOutput(); }
   bool pendingSend()       { return _connectionHandler->pendingSend();  }
   AMQPConnectionHandler * connectionHandler(){ return _connectionHandler; }

 private:
   ConnectionState                      _connectionState;
   BlockingQueue<RabbitMessageBase * >  _jobQueue;
   AMQPEventLoop *                      _eventLoop;
   AMQPConnectionHandler *              _connectionHandler;
   Heartbeat                            _heartbeat;

   static const int  _outgoingBufferHighWatermark = 4096;
};

} //namespace AMQP
#endif
