#ifndef BLOCKING_QUEUE
#define BLOCKING_QUEUE

#include <deque>
#include <condition_variable>
#include <mutex>
#include <boost/noncopyable.hpp>
#include <sys/eventfd.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>             /* Definition of uint64_t */


namespace AMQP
{

template< typename DataType >
using DisposeMethod = std::function< void ( DataType d )> ;

template< typename DataType >
    class QueueImpl : public boost::noncopyable
{
 public:
   QueueImpl() :
       _eventFD( eventfd( 0, EFD_SEMAPHORE ) )
    {}

   bool push( DataType & data, bool first )
   {
       if ( first )
       {
           _queue.push_front( data );
       } else {
           _queue.push_back( data );
       }
       uint64_t dummy = 1;
       write( _eventFD, &dummy, sizeof( dummy ) );
       return true;
   }

   bool pop( DataType & data )
   {
       if( _queue.empty() )
       {
           return false;
       }

       ssize_t dummy;
       read( _eventFD, & dummy, sizeof( dummy ) );
       data = _queue.front();
       _queue.pop_front();
       return true;
   }

   int getFD() const
   {
       return _eventFD;
   }

   bool empty() const
   {
       return _queue.empty();
   }

 private:
   std::deque<DataType>         _queue;
   int                          _eventFD;
};

template< typename DataType >
class BlockingQueue : public boost::noncopyable
{
 public:
   BlockingQueue( DisposeMethod< DataType > disposeMethod ):
       _queue( new QueueImpl< DataType > ),
       _disposeMethod( disposeMethod )
    {}

   ~BlockingQueue()
   {
       this->flush();
       delete _queue;
   }

   bool pushFront( DataType & data )
   { 
       return doPush ( data, true ); 
   }

   bool push( DataType data )
   { 
       return doPush ( data, false ); 
   }

   int getFD() const
   {
       return _queue->getFD();
   }

   bool empty() const
   {
       std::unique_lock< std::recursive_mutex > lock(_queueMutex);
       return _queue->empty();
   }

   void flush()
   {
       std::unique_lock< std::recursive_mutex > lock(_queueMutex);
       while( ! _queue->empty() )
       {
           DataType d;
           pop( d );
           _disposeMethod( d );
       }
       _queueEmptyCondition.notify_all();
   }

   bool try_pop( DataType & data )
   {
       std::unique_lock< std::recursive_mutex > lock(_queueMutex);
       return _queue->pop( data );
   }

   void pop( DataType & data )
   {
       std::unique_lock< std::recursive_mutex > lock(_queueMutex);
       while( _queue->empty() )
       {
           _queueEmptyCondition.wait( _queueMutex );
       }
       _queue->pop( data );
   }

 private:
   bool doPush( DataType & data, bool first )
   {
//TODO: yossi removed _queue->size() ??
//       int msgAmountToBeSent = _queue->size();
//       if ( msgAmountToBeSent > 10000 )
//       {
//            std::cout << "Blocking queue size is too big! size:" << msgAmountToBeSent << std::endl;
//       }
       
       {
           std::unique_lock< std::recursive_mutex > lock(_queueMutex);
           _queue->push( data, first );
       }
       _queueEmptyCondition.notify_all();
       return true;
   }

 private:
   std::recursive_mutex             _queueMutex;
   std::condition_variable_any      _queueEmptyCondition;
   QueueImpl< DataType > *          _queue;
   DisposeMethod< DataType >        _disposeMethod;
};

} //namespace AMQP
#endif
