#ifndef RABBIT_PROXY_H
#define RABBIT_PROXY_H

#include <boost/noncopyable.hpp>

#include "connectionDetails.h"
//TODO: delete the crap bellow
//this is here only till I'll get rid on the ctor that connects in AMQP.
//Once this is done, this whole class can go.

class AMQP;

class RabbitProxy : boost::noncopyable
{
 public:
    RabbitProxy (const connectionDetails& i_connectionDetails) :
        m_connectionHolder(nullptr),
        m_connectionDetails( i_connectionDetails),
        m_stop(false)
    { }
    bool connect();
    bool init();

    void stop();

    AMQP* m_connectionHolder;
 protected:
    connectionDetails m_connectionDetails;
 protected:
    volatile bool m_stop;
};
#endif
