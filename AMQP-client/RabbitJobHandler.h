#ifndef RABBIT_JOB_HANDLER
#define RABBIT_JOB_HANDLER

#include "Heartbeat.h"
#include "ConnectionState.h"

namespace AMQP {

class RabbitConnectionDetails;
class RabbitMessageBase;
class RabbitEventLoop;
class RabbitConnection;
class RabbitJobQueue;

class RabbitJobHandler
{
 public:
   using OnMessageReceivedCB = std::function<int( const AMQP::Message& )> ;

   RabbitJobHandler( OnMessageReceivedCB onMsgReceivedCB, RabbitJobQueue& jobQueue ) ;
   ~RabbitJobHandler( );

   DeferredResult addJob ( RabbitMessageBase * job );

   bool start( const RabbitConnectionDetails & connectionParams );
   void stopEventLoop( DeferredResultSetter returnValueSetter );
   bool canHandleMessage() const;
   void handleTimeout();
   bool handleInput() { _heartbeat.reset(); return _connection->handleInput();  }
   bool handleOutput() const     { return _connection->handleOutput(); }
   bool pendingSend() const       { return _connection->pendingSend();  }
   RabbitConnection * connectionHandler(){ return _connection; }
   void waitForDisconnection();

 protected:
   void doStart( const RabbitConnectionDetails & connectionParamsm, 
           DeferredResultSetter connectedReturnValueSetter );

 private:
   ConnectionState                      _connectionState;
   RabbitConnection *                   _connection;
   Heartbeat                            _heartbeat;
   RabbitJobQueue &                     _jobQueue;
   RabbitEventLoop *                    _eventLoop;
   std::thread                          _eventLoopThread;

   static const int  _outgoingBufferHighWatermark = 4096;
};

} //namespace AMQP
#endif
