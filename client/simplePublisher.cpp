#include "simplePublisher.h"
#include "simpleClient.h" //Remove, for ExchangeTypeStr
#include <AMQPcpp.h>

simplePublisher::simplePublisher(const connectionDetails& i_connectionDetails, 
    const std::string& i_exchangeName, 
    ExchangeType       i_exchangeType,
    const std::string& i_consumerID,
    BlockingQueue<RabbitMessageBase*>& i_messageQueueToSend):
  m_rabbitProxy(i_connectionDetails),
  m_consumerID(i_consumerID),
  m_messageQueueToSend(i_messageQueueToSend),
  m_runStatus(RunStatus::Continue),
  m_exchange(NULL),
  m_exchangeName(i_exchangeName),
  m_exchageType(i_exchangeType)
{}

void simplePublisher::operator()()
{
    RABBIT_DEBUG ("Publisher:: Publisher started ");
    while (1)
    {
        m_rabbitProxy.init();
        m_exchange = m_rabbitProxy.m_connectionHolder->createExchange(m_exchangeName);
        m_exchange->Declare(m_exchangeName, ExchangeTypeStr[ (int)m_exchageType ] );
        //TODO: find a nice way to do this.
        //Actually, I have an idea. I'll implement it soon.
        //Just want to get something working to keep Adam (adam@liveu.tv) Happy :-)
        m_exchange->setHeader("Content-type", "text/text");
        m_exchange->setHeader("Content-encoding", "UTF-8");
        m_exchange->setHeader("Delivery-mode", 1);

        RabbitMessageBase* pMessage = nullptr;
        //TODO: add normal/immediate stop logic
        try
        {
            for (;;)
            {
                m_messageQueueToSend.pop(pMessage);
                std::string routingKey= pMessage->getRoutingKey();
                m_exchange->Publish( pMessage->serialize(), routingKey );
                RABBIT_DEBUG("Publisher:: Going to publish message: " << *pMessage );
                delete pMessage;
                pMessage = nullptr;
            }
        } catch (AMQPException e) {
            //m_pMessage->ueueToSend.push_front(message); ??
            RABBIT_DEBUG ("Publisher:: got exception " << e.getMessage());

        }
    }
} 


void simplePublisher::stop(bool immediate)
{
    m_runStatus = (immediate) ? RunStatus::StopImmediate : RunStatus::StopGracefull;
}
