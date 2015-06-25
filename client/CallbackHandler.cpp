#include "CallbackHandler.h"

void CallbackHandler::start()
{
    _callbackThread = std::thread( std::bind( & CallbackHandler::doStart, this ) );
    _callbackThread.detach();
}

void CallbackHandler::addMessage( const std::string & sender,
        const std::string & destination,
        DeliveryType deliveryType,
        const std::string & text )
{
    MessageData * newMessage = new MessageData( sender, destination, deliveryType, text);
    _messageQueue.push( newMessage );
}

void CallbackHandler::doStart()
{
    while( true )
    {
        MessageData * messageToHandle = nullptr;
        _messageQueue.pop( messageToHandle );
        _onMessageReceivedCB( messageToHandle->_sender, 
                messageToHandle->_destination, 
                messageToHandle->_deliveryType, 
                messageToHandle->_text );
        //delete messageToHandle;
    }
}
