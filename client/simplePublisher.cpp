#include "simplePublisher.h"
#include "common.h"
#include <AMQPcpp.h>

simplePublisher::simplePublisher(const connectionDetails& i_connectionDetails, 
    const std::string& i_exchangeName, 
    ExchangeType       i_exchangeType,
    const std::string& i_consumerID,
    BlockingQueue<Protocol>& i_messageQueueToSend):
  m_rabbitProxy(i_connectionDetails),
  m_consumerID(i_consumerID),
  m_messageQueueToSend(i_messageQueueToSend),
  m_stopStatus(StopStatus::Continue),
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

        Protocol  message;
        //TODO: add normal/immediate stop logic
        try
        {
            for (;;)
            {
                m_messageQueueToSend.pop(message);
                RABBIT_DEBUG ("Publisher:: going to publish " << message.m_text <<" To: "<<message.m_destination);
                m_exchange->Publish(message.m_text, message.m_destination);
            }
        } catch (AMQPException e) {
            //m_messageQueueToSend.push_front(message); ??
            RABBIT_DEBUG ("Publisher:: got exception " << e.getMessage());

        }
    }
} 

void simplePublisher::stop(bool immediate)
{
    m_stopStatus = (immediate) ? StopStatus::StopImmediate : StopStatus::StopGracefull;
}
