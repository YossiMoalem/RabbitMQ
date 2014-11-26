#include "rabbitProxy.h"
#include "common.h"

#include <AMQPcpp.h>

int RabbitProxy::connect()
{
    for (;;)
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
            return 0;
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
      MessageQueue m_messageQueueReceived;
    }
}

int RabbitProxy::init()
{
    connect();
    return 0;
}

