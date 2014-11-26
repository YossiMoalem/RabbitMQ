#ifndef BLOCKING_QUEUE
#define BLOCKING_QUEUE

#include <deque>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

template<typename Data>
class BlockingQueue : public boost::noncopyable
{
public:
    void push(Data const& data)
    {
        boost::mutex::scoped_lock lock(m_queueMutex);
        m_queue.push_back(data);
        lock.unlock();
        the_condition_variable.notify_all();
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
};

#endif
