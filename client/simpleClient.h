#ifndef SIMPLE_CLIENT_H
#define SIMPLE_CLIENT_H

#include <boost/noncopyable.hpp>
#include <string>

#include "Types.h"
#include "connectionDetails.h"

class RabbitClientImpl;

class simpleClient : public boost::noncopyable
{
 public:
   simpleClient(const connectionDetails& i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           ExchangeType       i_exchangeType,
           RabbitMQNotifiableIntf* i_handler);

   simpleClient(const connectionDetails& i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           ExchangeType       i_exchangeType,
           CallbackType       i_onMessageCB );

   int start();
   int stop(bool immediate);

   int sendUnicast      (	const std::string& i_message,
							const std::string& i_destination,
							const std::string& i_senderID );
   int sendMulticast    (const std::string& i_message, const std::string& i_senderID);

   int bindToSelf           (const std::string& i_key);
   int bindToDestination    (const std::string& i_key);
   int unbindFromSelf       (const std::string& i_key);
   int unbindFromDestination(const std::string& i_key);

 private:
   RabbitClientImpl* m_pRabbitClient;

};


#endif
