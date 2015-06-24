#ifndef RABBIT_JOB_MANAGER
#define RABBIT_JOB_MANAGER

#include "ResultCodes.h"
#include "BlockingQueue.h"
#include "AMQPConnectionHandler.h"
#include "Heartbeat.h"
#include "ConnectionState.h"

#include <thread>

namespace AMQP {

class AMQPConnectionDetails;
class RabbitMessageBase;
class AMQPEventLoop;

class RabbitJobManager
{
 public:
   RabbitJobManager( std::function<int( const AMQP::Message& )> onMsgReceivedCB ) ;
   ~RabbitJobManager( );

   DeferedResult addJob ( RabbitMessageBase * job );
   bool start( const AMQPConnectionDetails & connectionParams );
   void stopEventLoop( bool immediate,
           DeferedResultSetter returnValueSetter );
   bool canHandleMessage() const;
   void handleTimeout();
   bool handleInput()       { _heartbeat.reset(); return _connectionHandler->handleInput();  }
   bool handleOutput()      { return _connectionHandler->handleOutput(); }
   bool pendingSend()       { return _connectionHandler->pendingSend();  }
   AMQPConnectionHandler * connectionHandler(){ return _connectionHandler; }
   void waitForDisconnection();

 protected:
   void terminate( );
   void startEventLoop( const AMQPConnectionDetails & connectionParamsm, DeferedResultSetter connectedReturnValueSetter );

 private:
   ConnectionState                      _connectionState;
   BlockingQueue<RabbitMessageBase * >  _jobQueue;
   AMQPConnectionHandler *              _connectionHandler;
   Heartbeat                            _heartbeat;
   AMQPEventLoop *                      _eventLoop;
   std::thread                          _eventLoopThread;

   static const int  _outgoingBufferHighWatermark = 4096;
};

} //namespace AMQP
#endif
