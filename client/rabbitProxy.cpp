#include "rabbitProxy.h"
#include "internalTypes.h"

#include <AMQPcpp.h>

#include <assert.h>

bool RabbitProxy::connect()
{
    m_connectionDetails.getFirstHost();
    while (!m_stop)
    {
      RABBIT_DEBUG ("RabbitProxy:: Attempting to connect");
        if( m_connectionHolder != NULL )
        {
            delete m_connectionHolder;
            m_connectionHolder = nullptr;
        }
        try{
            // Purely dis·gust·ing!! not a singler nice this about this interface
            // the instance should just be there, and I should call it's connect methos
            // providing connection DETAILS.
            m_connectionHolder = new AMQP(m_connectionDetails.createConnectionString());
            RABBIT_DEBUG ("RabbitProxy:: Connected ");
            return true;
        }
        catch ( AMQPException e )
        {
            if (m_connectionDetails.isLastHost())
            {
                RABBIT_DEBUG ("RabbitProxy:: Tried all possible hosts. Resting a bit ");
                sleep(2);
            }
            //TODO: this is just to advance the host pointer.
            (void)m_connectionDetails.getNextHost();
        }
    }
    assert (m_stop);
    RABBIT_DEBUG("RabbitProxy:: Got stop command. exiting");
    return false;
}

void RabbitProxy::stop ()
{
    m_stop = true;
}

bool RabbitProxy::init()
{
  return connect();
}

