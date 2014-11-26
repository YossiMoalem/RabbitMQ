#ifndef SIMPLE_CLIENT_H
#define SIMPLE_CLIENT_H

#include <boost/noncopyable.hpp>
#include <string>

#include "connectionDetails.h"
#include "simpleConsumer.h"


class AMQPMessage;
class RabbitClientImpl;

class RabbitMQNotifiableIntf 
{
 public:
  virtual int onMessageReceive (AMQPMessage* i_mas) = 0;
};

class simpleClient : public boost::noncopyable
{
 public:
   simpleClient(const connectionDetails& i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           RabbitMQNotifiableIntf* i_handler);

   simpleClient(const connectionDetails& i_connectionDetails, 
           const std::string& i_exchangeName, 
           const std::string& i_consumerID,
           int (*i_onMessageCB)(AMQPMessage*) );

   int start();
   int stop(bool immediate);

   int sendUnicast      (const std::string& i_message, const std::string& i_destination);
   int sendMulticast    (const std::string& i_message, const std::string& i_destination);

   int bindToSelf           (const std::string& i_key);
   int bindToDestination    (const std::string& i_key);
   int unbindFromSelf       (const std::string& i_key);
   int unbindFromDestination(const std::string& i_key);

 private:
   RabbitClientImpl* p_impl;

};


#endif
