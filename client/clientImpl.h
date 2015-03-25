#ifndef CLIENT_IMPL_H
#define CLIENT_IMPL_H

#include <boost/noncopyable.hpp>
#include <thread>
#include <string>

#include "ConnectionDetails.h"
#include "simpleConsumer.h"
#include "simplePublisher.h"

class RabbitClientImpl : public boost::noncopyable
{
 public:
   RabbitClientImpl(const ConnectionDetails & i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           RabbitMQNotifiableIntf* i_handler) ; 

   RabbitClientImpl(const ConnectionDetails & i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           CallbackType       i_onMessageCB );

   int start();
   int stop(bool immediate);

   ReturnStatus sendMessage(const std::string& i_message, 
       const std::string& i_destination, 
       const std::string& i_senderID, 
       DeliveryType i_deliveryType);

   ReturnStatus bind(const std::string& i_key, DeliveryType i_deliveryType);
   ReturnStatus unbind(const std::string& i_key, DeliveryType i_deliveryType);
   ReturnStatus sendMessage(BindMessage*   i_bindMessage);
   ReturnStatus sendMessage(UnbindMessage* i_unbindMessage);
   ReturnStatus sendMessage(PostMessage*   i_postBMessage);

 private:
   ReturnStatus doSendMessage(RabbitMessageBase* i_message, bool highPriority);

 private:
   const ConnectionDetails  m_connectionDetails;
   const std::string        m_exchangeName;
   const std::string        m_consumerID;
   CallbackType             m_onMessageCB;
   RabbitMQNotifiableIntf*  m_handler;
   MessageQueue             m_messageQueueToSend;
   simplePublisher          m_publisher;
   simpleConsumer           m_consumer;
   std::thread            m_consumerThread;
   std::thread            m_publisherThread;
};


#endif
