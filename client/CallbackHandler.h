#ifndef CALLBACK_HANDLER_H
#define CALLBACK_HANDLER_H

#include "Types.h"

#include <BlockingQueue.h>

#include <string>
#include <thread>

class CallbackHandler
{
 public:
   CallbackHandler( CallbackType onMessageReceivedCB ) :
       _onMessageReceivedCB( onMessageReceivedCB ),
       _messageQueue( []( MessageData * message ) { delete message; } )
    {}
   void start();
   void stop();
   void addMessage( const std::string &  sender,
           const std::string &  destination,
           DeliveryType         deliveryType,
           const std::string &  text );

 private:
   struct MessageData
   {
       MessageData( const std::string  & sender,
               const std::string  & destination,
               DeliveryType deliveryType,
               const std::string  & text ) :
           _sender          ( sender ),
           _destination     ( destination ),
           _text            ( text ),
           _deliveryType    ( deliveryType )
       {}

       std::string  _sender;
       std::string  _destination;
       std::string  _text;
       DeliveryType _deliveryType;
   };

 private:
   void doStart();

 private:
   CallbackType                         _onMessageReceivedCB;
   AMQP::BlockingQueue< MessageData * > _messageQueue;
   std::thread                          _callbackThread;

};

#endif
