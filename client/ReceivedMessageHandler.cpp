#include "ReceivedMessageHandler.h"

void ReceivedMessageHandler::start()
{
    _callbackThread = std::thread( std::bind( & ReceivedMessageHandler::doStart, this ) );
    _callbackThread.detach();
}

void ReceivedMessageHandler::addMessage( const std::string & sender,
        const std::string & destination,
        DeliveryType deliveryType,
        const std::string & text )
{
    MessageData * newMessage = new MessageData( sender, destination, deliveryType, text);
    _messageQueue.pushBack( newMessage );
}

void ReceivedMessageHandler::doStart()
{
    while( true )
    {
        MessageData * messageToHandle = nullptr;
        _messageQueue.pop( messageToHandle );
        _onMessageReceivedCallback( messageToHandle->sender, 
                messageToHandle->destination, 
                messageToHandle->deliveryType, 
                messageToHandle->text );
        delete messageToHandle;
    }
}
