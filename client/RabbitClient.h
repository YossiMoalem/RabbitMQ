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
   RabbitClient(const ConnectionDetails & connectionDetails, 
		   const std::string & exchangeName,
		   const std::string & consumerID,
		   CallbackType       onMessageCallback );

   ReturnStatus start();
   ReturnStatus stop(bool immediate);

   ReturnStatus sendUnicast      (	const std::string & message,
		   const std::string & destination,
		   const std::string & senderID,
		   const std::string & exchangeName);
   ReturnStatus sendMulticast    (const std::string & message,
		   const std::string & senderID,
		   const std::string & exchangeName);

   ReturnStatus bindToSelf           (const std::string & key);
   ReturnStatus bindToDestination    (const std::string & key);
   ReturnStatus unbindFromSelf       (const std::string & key);
   ReturnStatus unbindFromDestination(const std::string & key);
   bool connected() const; 

 private:
   RabbitClientImpl* _rabbitClient;

};


#endif
