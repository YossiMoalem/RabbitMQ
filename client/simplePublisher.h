#ifndef SIMPLE_PUBLISHER_H
#define SIMPLE_PUBLISHER_H

#include <string>

#include "RabbitMessage.h"
#include "rabbitProxy.h"
#include "BlockingQueue.h"
#include <myConnectionHandler.h>

enum class ExchangeType;
class ConnectionDetails;

class simplePublisher : boost::noncopyable
{
 public:
   simplePublisher( const ConnectionDetails& i_connectionDetails, 
       const std::string& i_exchangeName, 
       ExchangeType       i_exchangeType,
       const std::string& i_consumerID,
       MessageQueue& m_messageQueueToSend
       );

   virtual void operator ()();
   virtual void stop(bool immediate);

 private:
   MyConnectionHandler          _connH;
   const std::string  m_consumerID;
   MessageQueue&      m_messageQueueToSend;
   RunStatus          m_runStatus;
   const std::string  m_exchangeName;
   ExchangeType       m_exchageType;
};

#endif
