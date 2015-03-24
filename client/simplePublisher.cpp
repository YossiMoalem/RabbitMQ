#include "simplePublisher.h"
#include "simpleClient.h" //Remove, for ExchangeTypeStr
#include <AmqpConnectionDetails.h>

simplePublisher::simplePublisher(const ConnectionDetails& i_connectionDetails, 
    const std::string& i_exchangeName, 
    ExchangeType       i_exchangeType,
    const std::string& i_consumerID,
    MessageQueue& i_messageQueueToSend):
  _connectionDetails( i_connectionDetails ),
  _connH( [this] (const AMQP::Message & i_message) { return 0; } ),
  m_consumerID(i_consumerID),
  m_messageQueueToSend(i_messageQueueToSend),
  m_runStatus(RunStatus::Continue),
  m_exchangeName(i_exchangeName),
  m_exchageType(i_exchangeType)
{}

void simplePublisher::operator()()
{
    RABBIT_DEBUG ("Publisher:: Publisher started ");
    while (1)
    {
        if ( _connH.login( _connectionDetails.getFirstHost() ) == false )
        {
            RABBIT_DEBUG("Publisher:: Publisher failed to (re)connect. Exiting. ");
            return;
        }
        _connH.declareExchange(m_exchangeName.c_str() /*, ExchangeTypeStr[ (int)m_exchageType ]a*/ );

        //TODO: find a nice way to do this.
        //Actually, I have an idea. I'll implement it soon.
        //Just want to get something working to keep Adam (adam@liveu.tv) Happy :-)
        /*m_exchange->setHeader("Content-type", "text/text");
        m_exchange->setHeader("Content-encoding", "UTF-8");
        m_exchange->setHeader("Delivery-mode", 1);
        */

        m_messageQueueToSend.setQueueState(MessageQueue::QueueState::QueueOpen);

        RabbitMessageBase* pMessage = nullptr;
        try
        {
            for (;;)
            {
                m_messageQueueToSend.pop(pMessage);
                if(pMessage == nullptr || m_runStatus == RunStatus::StopImmediate)
                {
                    RABBIT_DEBUG("Publisher:: either, got stopImmediat, or, "
                            <<" got stop, and no more messages in the queue. Exiting");
                    assert (m_runStatus != RunStatus::Continue);
                    return;
                }
                std::string routingKey= pMessage->getRoutingKey();
                RABBIT_DEBUG("Publisher:: Going to publish message: " << *pMessage << "to key: " << routingKey );
                _connH.publish( routingKey.c_str(),  pMessage->serialize().c_str());
                delete pMessage;
                pMessage = nullptr;
            }
        } catch ( ... ) {
            m_messageQueueToSend.setQueueState(MessageQueue::QueueState::AdminOnly);
            MessageType messageType = pMessage->messageType();
            if (messageType == MessageType::Bind||
                    messageType == MessageType::Unbind)
            {

                m_messageQueueToSend.pushFront(pMessage, true);
            }
           // RABBIT_DEBUG ("Publisher:: got exception " << e.getMessage());
        }
    }
} 


void simplePublisher::stop(bool immediate)
{
    m_runStatus = (immediate) ? RunStatus::StopImmediate : RunStatus::StopGracefull;
    //m_rabbitProxy.stop();
    m_messageQueueToSend.terminate(immediate);
}
