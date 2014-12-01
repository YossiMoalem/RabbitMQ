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
  virtual int onMessageReceive (std::string o_message, std::string o_sender, DeliveryType o_deliveryType) = 0;
};

enum class ExchangeType
{
    Direct,
    Topic,
    Fanout,
    Last
};
//TODO: does not really needs to be here
static const char* const ExchangeTypeStr[ (int)ExchangeType::Last ] = {"direct", "topic", "fanout"};

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

   int sendUnicast      (const std::string& i_message, const std::string& i_destination, const std::string& i_senderID);
   int sendMulticast    (const std::string& i_message, const std::string& i_destination, const std::string& i_senderID);

   int bindToSelf           (const std::string& i_key);
   int bindToDestination    (const std::string& i_key);
   int unbindFromSelf       (const std::string& i_key);
   int unbindFromDestination(const std::string& i_key);

 private:
   RabbitClientImpl* m_pRabbitClient;

};


#endif
