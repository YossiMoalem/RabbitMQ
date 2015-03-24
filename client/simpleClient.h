#ifndef SIMPLE_CLIENT_H
#define SIMPLE_CLIENT_H

#include <boost/noncopyable.hpp>
#include <string>

#include "Types.h"
#include "ConnectionDetails.h"

class RabbitClientImpl;

class simpleClient : public boost::noncopyable
{
 public:
   simpleClient(const ConnectionDetails & i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           ExchangeType       i_exchangeType,
           RabbitMQNotifiableIntf* i_handler);

   simpleClient(const ConnectionDetails & i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           ExchangeType       i_exchangeType,
           CallbackType       i_onMessageCB );

   int start();
   int stop(bool immediate);

   ReturnStatus sendUnicast      (	const std::string& i_message,
							const std::string& i_destination,
							const std::string& i_senderID );
   ReturnStatus sendMulticast    (const std::string& i_message, const std::string& i_senderID);

   ReturnStatus bindToSelf           (const std::string& i_key);
   ReturnStatus bindToDestination    (const std::string& i_key);
   ReturnStatus unbindFromSelf       (const std::string& i_key);
   ReturnStatus unbindFromDestination(const std::string& i_key);

 private:
   RabbitClientImpl* m_pRabbitClient;

};


#endif
