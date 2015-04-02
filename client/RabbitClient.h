#ifndef SIMPLE_CLIENT_H
#define SIMPLE_CLIENT_H

#include <boost/noncopyable.hpp>
#include <string>

#include "Types.h"
#include "ConnectionDetails.h"

class RabbitClientImpl;

class RabbitClient : public boost::noncopyable
{
 public:
   RabbitClient(const ConnectionDetails & i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           CallbackType       i_onMessageCB );

   ReturnStatus start();
   ReturnStatus stop(bool immediate);

   ReturnStatus sendUnicast      (	const std::string& i_message,
							const std::string& i_destination,
							const std::string& i_senderID );
   ReturnStatus sendMulticast    (const std::string& i_message, const std::string& i_senderID);

   ReturnStatus bindToSelf           (const std::string& i_key);
   ReturnStatus bindToDestination    (const std::string& i_key);
   ReturnStatus unbindFromSelf       (const std::string& i_key);
   ReturnStatus unbindFromDestination(const std::string& i_key);
   bool isConnected() const; 

 private:
   RabbitClientImpl* m_pRabbitClient;

};


#endif
