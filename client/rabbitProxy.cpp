#include "rabbitProxy.h"
#include "internalTypes.h"

#include <AMQPcpp.h>

#include <assert.h>

bool RabbitProxy::connect()
{
    while (!m_stop)
    {
      RABBIT_DEBUG ("RabbitProxy:: Attempting to connect");
        if( m_connectionHolder != NULL )
        {
            delete m_connectionHolder;
            m_connectionHolder = nullptr;
        }
        //TODO: this is just to reset the host pointer.
        //in normal impl, I'd use the rev ral, and use it to connect.
        m_connectionDetails.getFirstHost();
        try{
            // Purely dis·gust·ing!! not a singler nice this about this interface
            // the instance should just be there, and I should call it's connect methos
            // providing connection DETAILS.
            m_connectionHolder = new AMQP( connectionDetails::createConnectionString(m_connectionDetails));
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
            //in normal impl, I'd use the rev ral, and use it to connect.
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

