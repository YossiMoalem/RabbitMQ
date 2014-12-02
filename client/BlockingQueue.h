#ifndef BLOCKING_QUEUE
#define BLOCKING_QUEUE

#include <deque>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

template<typename Data>
class BlockingQueue : public boost::noncopyable
{
public:
    enum class ReturnStatus
    {
      Ok,
      QueueOpenForHigPriorityDataOnly,
      QueueBlocked
    };

    enum class QueueState
    {
      QueueOpen,
      HighPriorityDataOnly,
      QueueBlocked
    };

    BlockingQueue () : m_queueState(QueueState::QueueOpen) 
    {}

    void setQueueState( QueueState i_newState )
    {
      m_queueState = i_newState;
    }

    ReturnStatus push(Data const& data, bool highPriority = false ) 
    {
        boost::mutex::scoped_lock lock(m_queueMutex);
        if(m_queueState == QueueState::QueueOpen || 
            ( m_queueState == QueueState::HighPriorityDataOnly && highPriority ) )
        {
          m_queue.push_back(data);
          lock.unlock();
          the_condition_variable.notify_all();
          return ReturnStatus::Ok;
        }
        if( m_queueState == QueueState::HighPriorityDataOnly )
        {
          assert (!highPriority);
          return ReturnStatus::QueueOpenForHigPriorityDataOnly;
        }
        assert (m_queueState == QueueState::QueueBlocked);
        return ReturnStatus::QueueBlocked;
    }

    bool empty() const
    {
        boost::mutex::scoped_lock lock(m_queueMutex);
        return m_queue.empty();
    }

    bool try_pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(m_queueMutex);
        if(m_queue.empty())
        {
            return false;
        }
        
        popped_value=m_queue.front();
        m_queue.pop_front();
        return true;
    }

    void pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(m_queueMutex);
        while(m_queue.empty())
        {
            the_condition_variable.wait(lock);
        }
        
        popped_value=m_queue.front();
        m_queue.pop_front();
    }

private:
    std::deque<Data> m_queue;
    mutable boost::mutex m_queueMutex;
    boost::condition_variable the_condition_variable;
    QueueState m_queueState;
};
#endif
