#ifndef RABBIT_JOB_MANAGER
#define RABBIT_JOB_MANAGER

#include "RabbitOperation.h"
#include "BlockingQueue.h"

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
   bool connect(const AMQPConnectionDetails & connectionParams );

 private:
   std::function<int( const AMQP::Message& )> _onMsgReceivedBC;
   BlockingQueue<RabbitMessageBase * >  _jobQueue;
   AMQPEventLoop *                  _eventLoop;
   AMQPConnectionHandler *              _connectionHandler;
};

} //namespace AMQP
#endif
