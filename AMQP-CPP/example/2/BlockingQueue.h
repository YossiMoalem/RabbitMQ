#ifndef BLOCKING_QUEUE
#define BLOCKING_QUEUE

#include <deque>
#include <condition_variable>
#include <mutex>
#include <boost/noncopyable.hpp>
#include <assert.h>


namespace AMQP
{
enum class RunStatus
{
    Continue = 0,
        StopGracefull = 1,
        StopImmediate = 2 
};
template< typename DataType >
class BlockingQueue : public boost::noncopyable
{
 public:
   enum class ReturnStatus
   {
       Ok,
       QueueOpenForAdminMessagesOnly,
       QueueBlocked
   };

   enum class QueueState
   {
       QueueOpen,
       AdminOnly,
       QueueBlocked
   };

   BlockingQueue () : 
       _queueState( QueueState::QueueOpen ),
       _runStatus( RunStatus::Continue )
    {}

   ReturnStatus pushFront( DataType const& i_data, bool adminMessage = false )
   { 
       return doPush ( i_data, adminMessage, true ); 
   }

   ReturnStatus push( DataType const& i_data, bool adminMessage = false )
   { 
       return doPush ( i_data, adminMessage, false ); 
   }

   //Not Nice..
   void setQueueState( QueueState i_newState )
   { 
       _queueState = i_newState; 
   }

   bool empty() const;
   void terminate( bool immediate );
   bool try_pop( DataType & o_data );
   void pop( DataType & o_data );

 private:
   ReturnStatus doPush( DataType const& i_data, bool adminMessage, bool forceFirst ) ;

 private:
   std::deque<DataType>         _queue;
   mutable std::mutex           _queueMutex;
   std::condition_variable      _queueEmptyCondition;
   QueueState                   _queueState;
   RunStatus                    _runStatus;
};

    template<typename DataType>
void BlockingQueue<DataType>::pop(DataType& o_data)
{
    std::unique_lock< std::mutex > lock(_queueMutex);
    while( _queue.empty() && _runStatus == RunStatus::Continue )
    {
        _queueEmptyCondition.wait(lock);
    }

    if (_runStatus == RunStatus::StopImmediate)
    {
        while (!_queue.empty())
        {
            delete (_queue.front());
            _queue.pop_front();
        }
        o_data = nullptr;
    } else if ( _queue.empty() ) {
        assert (_runStatus == RunStatus::StopGracefull);
        o_data = nullptr;
    } else {
        o_data=_queue.front();
        _queue.pop_front();
    }
}
    template<typename DataType>
bool BlockingQueue<DataType>::try_pop(DataType& o_data)
{
    std::unique_lock< std::mutex > lock(_queueMutex);
    if(_queue.empty())
    {
        return false;
    }

    o_data=_queue.front();
    _queue.pop_front();
    return true;
}

    template<typename DataType>
void BlockingQueue<DataType>::terminate (bool immediate)
{
    std::unique_lock< std::mutex > lock(_queueMutex);
    _queueState = QueueState::QueueBlocked;
    _runStatus = (immediate) ? RunStatus::StopImmediate : RunStatus::StopGracefull;
    _queueEmptyCondition.notify_all();
}

template<typename DataType>
bool BlockingQueue<DataType>::empty() const
{
    std::unique_lock< std::mutex > lock(_queueMutex);
    return _queue.empty();
}


template<typename DataType>
auto BlockingQueue<DataType>::doPush(DataType const& i_data, bool adminMessage, bool forceFirst ) -> ReturnStatus
{
    std::unique_lock< std::mutex > lock(_queueMutex);
    if(_queueState == QueueState::QueueOpen ||
            ( _queueState == QueueState::AdminOnly && adminMessage) )
    {
        if (forceFirst)
        {
            _queue.push_front(i_data);
        } else {
            _queue.push_back(i_data);
        }
        lock.unlock();
        _queueEmptyCondition.notify_all();
        return ReturnStatus::Ok;
    }
    if( _queueState == QueueState::AdminOnly )
    {
        assert (!adminMessage);
        return ReturnStatus::QueueOpenForAdminMessagesOnly;
    }
    assert (_queueState == QueueState::QueueBlocked);
    return ReturnStatus::QueueBlocked;
}

} //namespace AMQP
#endif
