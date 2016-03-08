#include "RabbitJobQueue.h"
#include "RabbitJobHandler.h"
#include "RabbitOperation.h"

namespace AMQP {

RabbitJobQueue::RabbitJobQueue( ):
    _jobQueue( []( RabbitMessageBase* message ) { delete message; } )
{}

DeferredResult RabbitJobQueue::addJob ( RabbitMessageBase * job )
{
    auto result = job->deferedResult();
    job->setHandler( _jobHandler );
    _jobQueue.push( job );
    return result;
}

DeferredResult RabbitJobQueue::addJobToFront ( RabbitMessageBase * job )
{
    auto result = job->deferedResult();
    job->setHandler( _jobHandler );
    _jobQueue.pushFront( job );
    return result;
}

bool RabbitJobQueue::tryPop( RabbitMessageBase *& message )
{
    return _jobQueue.try_pop( message );
}

void RabbitJobQueue::clear()
{
    _jobQueue.clear();
}

int RabbitJobQueue::getFD() const
{
    return _jobQueue.getFD();
}

void RabbitJobQueue::setHandler( RabbitJobHandler * jobHandler )
{
    _jobHandler = jobHandler;
}

} //namespace AMQP

