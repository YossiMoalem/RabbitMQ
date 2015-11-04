#ifndef RABBIT_JOB_QUEUE
#define RABBIT_JOB_QUEUE

#include "Types.h"
#include "BlockingQueue.h"

namespace AMQP
{
class RabbitMessageBase;
class RabbitJobHandler;

class RabbitJobQueue
{
 public:
   RabbitJobQueue( );
   DeferedResult addJob( RabbitMessageBase * job );
   DeferedResult addJobToFront( RabbitMessageBase * job );
   bool tryPop( RabbitMessageBase *& message );
   void clear();
   int getFD() const;
   void setHandler( RabbitJobHandler * jobHandler );

 private:
   BlockingQueue< RabbitMessageBase * >  _jobQueue;
   RabbitJobHandler *                    _jobHandler;
};
} //namespace AMQP
#endif //RABBIT_JOB_QUEUE
