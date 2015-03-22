#ifndef BLOCKING_QUEUE
#define BLOCKING_QUEUE

#include <deque>
#include <condition_variable>
#include <mutex>
#include <boost/noncopyable.hpp>
#include <assert.h>

template<typename Data>
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

   BlockingQueue () : m_queueState(QueueState::QueueOpen),
    m_runStatus(RunStatus::Continue)
    {}
   BlockingQueue<Data>(const BlockingQueue&) = delete;
   BlockingQueue<Data>& operator=(const BlockingQueue<Data>) = delete;

   ReturnStatus pushFront(Data const& i_data, bool adminMessage = false )
   { return doPush (i_data, adminMessage, true); }

   ReturnStatus push(Data const& i_data, bool adminMessage = false )
   { return doPush (i_data, adminMessage, false); }

   void setQueueState( QueueState i_newState )
   { m_queueState = i_newState; }

   bool empty() const;
   void terminate (bool immediate);
   bool try_pop(Data& o_data);
   void pop(Data& o_data);

 private:
   ReturnStatus doPush(Data const& i_data, bool adminMessage, bool forceFirst ) ;

 private:
   std::deque<Data>             m_queue;
   mutable std::mutex           m_queueMutex;
   std::condition_variable      m_queueEmptyCondition;
   QueueState                   m_queueState;
   RunStatus                    m_runStatus;
};

    template<typename Data>
void BlockingQueue<Data>::pop(Data& o_data)
{
    std::unique_lock< std::mutex > lock(m_queueMutex);
    while( m_queue.empty() && m_runStatus == RunStatus::Continue )
    {
        m_queueEmptyCondition.wait(lock);
    }

    if (m_runStatus == RunStatus::StopImmediate)
    {
        while (!m_queue.empty())
        {
            delete (m_queue.front());
            m_queue.pop_front();
        }
        o_data = nullptr;
    } else if ( m_queue.empty() ) {
        assert (m_runStatus == RunStatus::StopGracefull);
        o_data = nullptr;
    } else {
        o_data=m_queue.front();
        m_queue.pop_front();
    }
}
    template<typename Data>
bool BlockingQueue<Data>::try_pop(Data& o_data)
{
    std::unique_lock< std::mutex > lock(m_queueMutex);
    if(m_queue.empty())
    {
        return false;
    }

    o_data=m_queue.front();
    m_queue.pop_front();
    return true;
}

    template<typename Data>
void BlockingQueue<Data>::terminate (bool immediate)
{
    std::unique_lock< std::mutex > lock(m_queueMutex);
    m_queueState = QueueState::QueueBlocked;
    m_runStatus = (immediate) ? RunStatus::StopImmediate : RunStatus::StopGracefull;
    m_queueEmptyCondition.notify_all();
}

template<typename Data>
bool BlockingQueue<Data>::empty() const
{
    std::unique_lock< std::mutex > lock(m_queueMutex);
    return m_queue.empty();
}


template<typename Data>
auto BlockingQueue<Data>::doPush(Data const& i_data, bool adminMessage, bool forceFirst ) -> ReturnStatus
{
    std::unique_lock< std::mutex > lock(m_queueMutex);
    if(m_queueState == QueueState::QueueOpen ||
            ( m_queueState == QueueState::AdminOnly && adminMessage) )
    {
        if (forceFirst)
        {
            m_queue.push_front(i_data);
        } else {
            m_queue.push_back(i_data);
        }
        lock.unlock();
        m_queueEmptyCondition.notify_all();
        return ReturnStatus::Ok;
    }
    if( m_queueState == QueueState::AdminOnly )
    {
        assert (!adminMessage);
        return ReturnStatus::QueueOpenForAdminMessagesOnly;
    }
    assert (m_queueState == QueueState::QueueBlocked);
    return ReturnStatus::QueueBlocked;
}

#endif
