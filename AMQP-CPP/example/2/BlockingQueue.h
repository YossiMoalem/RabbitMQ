#ifndef BLOCKING_QUEUE
#define BLOCKING_QUEUE

#include <deque>
#include <condition_variable>
#include <mutex>
#include <boost/noncopyable.hpp>
#include <assert.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>             /* Definition of uint64_t */


namespace{
template< typename T >
    void dispose( T t ) {}
template< typename T >
    void dispose( T * t )
    {
        delete t;
    }
}//namespace

namespace AMQP
{
template< typename DataType >
class BlockingQueue : public boost::noncopyable
{
 public:

   BlockingQueue () : 
       _queueOpen( true ),
       _eventFD( eventfd( 0, EFD_SEMAPHORE ) )
    {}

   ~BlockingQueue()
   {
       this->flush();
   }

   bool pushFront( DataType const& i_data )
   { 
       return doPush ( i_data, true ); 
   }

   bool push( DataType const& i_data )
   { 
       return doPush ( i_data, false ); 
   }

   int getFD() const
   {
        return _eventFD;
   }

   void close()
   {
       _queueOpen = false;
   }

   bool empty() const;
   void flush( );
   bool try_pop( DataType & data );
   void pop( DataType & data );

 private:
   bool doPush( DataType const& i_data, bool forceFirst ) ;

 private:
   std::deque<DataType>         _queue;
   mutable std::mutex           _queueMutex;
   std::condition_variable      _queueEmptyCondition;
   bool                         _queueOpen;
   int                          _eventFD;
};

    template<typename DataType>
void BlockingQueue< DataType >::pop( DataType & data )
{
    std::unique_lock< std::mutex > lock(_queueMutex);
    while( _queue.empty() && _queueOpen )
    {
        _queueEmptyCondition.wait(lock);
    }

    if( ! _queue.empty )
    {
        ssize_t dummy;
        read( _eventFD, & dummy, sizeof( dummy ) );
        data = _queue.front();
        _queue.pop_front();
    }
}

    template<typename DataType>
bool BlockingQueue< DataType >::try_pop( DataType & data )
{
    std::unique_lock< std::mutex > lock(_queueMutex);
    if( _queue.empty() )
    {
        return false;
    }

    ssize_t dummy;
    read( _eventFD, & dummy, sizeof( dummy ) );
    data=_queue.front();
    _queue.pop_front();
    return true;
}

    template<typename DataType >
void BlockingQueue<DataType>::flush()
{
    std::unique_lock< std::mutex > lock(_queueMutex);
    //make sure...
    _queueOpen = false;
    while( ! _queue.empty() )
    {
        DataType d = _queue.front();
        _queue.pop_front();
        dispose( d );
    }
    //If there is any client still waiting.
    _queueEmptyCondition.notify_all();
}

template<typename DataType>
bool BlockingQueue<DataType>::empty() const
{
    std::unique_lock< std::mutex > lock(_queueMutex);
    return _queue.empty();
}

template<typename DataType>
bool BlockingQueue<DataType>::doPush(DataType const& i_data, bool forceFirst ) 
{
    std::unique_lock< std::mutex > lock(_queueMutex);
    if( _queueOpen )
    {
        if (forceFirst)
        {
            _queue.push_front(i_data);
        } else {
            _queue.push_back(i_data);
        }
        uint64_t dummy = 1;
        write( _eventFD, &dummy, sizeof( dummy ) );
        lock.unlock();
        _queueEmptyCondition.notify_all();
        return true;
    }
    assert ( ! _queueOpen );
    return false;
}


} //namespace AMQP
#endif
