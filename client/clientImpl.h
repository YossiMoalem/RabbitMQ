#ifndef CLIENT_IMPL_H
#define CLIENT_IMPL_H

#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <string>

#include "connectionDetails.h"
#include "simpleConsumer.h"
#include "simplePublisher.h"

class AMQPMessage;
class AMQPExchange;
class AMQP;
class AMQPQueue;

class RabbitClientImpl : public boost::noncopyable
{
 public:
   RabbitClientImpl(const connectionDetails& i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           ExchangeType       i_exchangeType,
           RabbitMQNotifiableIntf* i_handler) ; 

   RabbitClientImpl(const connectionDetails& i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           ExchangeType       i_exchangeType,
           CallbackType       i_onMessageCB );

   int start();
   int stop(bool immediate);

   int sendMessage(const std::string& i_message, 
       const std::string& i_destination, 
       const std::string& i_senderID, 
       DeliveryType i_deliveryType);

   int bind(const std::string& i_key, DeliveryType i_deliveryType);
   int unbind(const std::string& i_key, DeliveryType i_deliveryType);
   int sendRawMessage (RabbitMessageBase* i_message);

 private:
   const connectionDetails m_connectionDetails;
   const std::string m_exchangeName;
   const std::string m_consumerID;
   CallbackType      m_onMessageCB;
   RabbitMQNotifiableIntf* m_handler;
   BlockingQueue<RabbitMessageBase*> m_messageQueueToSend;
   simplePublisher m_publisher;
   simpleConsumer m_consumer;
   boost::thread_group m_threads;
};


#endif
