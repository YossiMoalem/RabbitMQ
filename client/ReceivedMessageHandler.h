#ifndef RECEIVED_MESSAGE_HANDLER_H
#define RECEIVED_MESSAGE_HANDLER_H

#include "Types.h"
#include <BlockingQueue.h>

#include <string>
#include <thread>
#include <boost/noncopyable.hpp>

class ReceivedMessageHandler
{
 public:
   ReceivedMessageHandler( HandleMessageCallback_t onMessageReceivedCallbacl ) :
       _onMessageReceivedCallback( onMessageReceivedCallbacl )
    {}
   void start();
   void stop();
   void addMessage( const std::string &  sender,
           const std::string &  destination,
           DeliveryType         deliveryType,
           const std::string &  text );

 private:
   struct MessageData : boost::noncopyable
   {
       MessageData( const std::string  & sender,
               const std::string  & destination,
               DeliveryType deliveryType,
               const std::string  & text ) :
           sender          ( sender ),
           destination     ( destination ),
           text            ( text ),
           deliveryType    ( deliveryType )
       {}

       std::string  sender;
       std::string  destination;
       std::string  text;
       DeliveryType deliveryType;
   };

 private:
   void doStart();

 private:
   HandleMessageCallback_t                          _onMessageReceivedCallback;
   AMQP::SelectableBlockingQueue< MessageData * >   _messageQueue;
   std::thread                                      _callbackThread;

};

#endif
