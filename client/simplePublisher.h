#ifndef SIMPLE_PUBLISHER_H
#define SIMPLE_PUBLISHER_H

#include <string>

#include "common.h"
#include "connectionDetails.h"
#include "rabbitProxy.h"

class AMQPExchange;

class simplePublisher : boost::noncopyable
{
 public:
   simplePublisher( const connectionDetails& i_connectionDetails, 
       const std::string& i_exchangeName, 
       const std::string& i_consumerID,
       MessageQueue& m_messageQueueToSend
       );

   virtual void operator ()();
   virtual void stop(bool immediate);

 private:
   RabbitProxy m_rabbitProxy;
   const std::string m_consumerID;
   MessageQueue& m_messageQueueToSend;
   StopStatus m_stopStatus;
   AMQPExchange* m_exchange ;
   const std::string m_exchangeName;
};

#endif
