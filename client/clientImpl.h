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
           int (*i_onMessageCB)(AMQPMessage*) );

   int start();
   int stop(bool immediate);

   int sendUnicast(const std::string& i_message, const std::string& i_destination);
   int sendMulticast(const std::string& i_message, const std::string& i_destination);

   int bindToSelf(const std::string& i_key);
   int bindToDestination(const std::string& i_key);
   int unbindFromSelf(const std::string& i_key);
   int unbindFromDestination(const std::string& i_key);

 private:
   int send(const std::string& i_message, 
           const std::string& i_destination) ;

 private:
   const connectionDetails m_connectionDetails;
   const std::string m_exchangeName;
   const std::string m_consumerID;
   int (*m_onMessageCB)(AMQPMessage*) ;
   RabbitMQNotifiableIntf* m_handler;
   BlockingQueue<Protocol> m_messageQueueToSend;
   simplePublisher m_publisher;
   simpleConsumer m_consumer;
   boost::thread_group m_threads;
};


#endif
