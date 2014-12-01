#ifndef SIMPLE_PUBLISHER_H
#define SIMPLE_PUBLISHER_H

#include <string>

#include "RabbitMessage.h"
#include "connectionDetails.h"
#include "rabbitProxy.h"
#include "BlockingQueue.h"

class AMQPExchange;
enum class ExchangeType;

class simplePublisher : boost::noncopyable
{
 public:
   simplePublisher( const connectionDetails& i_connectionDetails, 
       const std::string& i_exchangeName, 
       ExchangeType       i_exchangeType,
       const std::string& i_consumerID,
       BlockingQueue<RabbitMessageBase*>& m_messageQueueToSend
       );

   virtual void operator ()();
   virtual void stop(bool immediate);

 private:
   RabbitProxy m_rabbitProxy;
   const std::string m_consumerID;
   BlockingQueue<RabbitMessageBase*>& m_messageQueueToSend;
   RunStatus m_runStatus;
   AMQPExchange* m_exchange ;
   const std::string m_exchangeName;
   ExchangeType                    m_exchageType;
};

#endif
