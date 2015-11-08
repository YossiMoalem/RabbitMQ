#include "RabbitConnectionHandler.h"
#include "Debug.h"
#include "RabbitSocket.h"
#include "ConnectionState.h"

#include <memory>

namespace AMQP {

RabbitConnectionHandler::RabbitConnectionHandler( OnMessageReceivedCB onMsgReceivedCB,
        ConnectionState & connectionState,
        std::shared_ptr< RabbitSocket > socket ) :
    _socket( socket ),
    _connectionState( connectionState )
{ }

void RabbitConnectionHandler::doPublish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message, 
        DeferedResultSetter operationSucceeded ) const
{
    _channel->publish( exchangeName, routingKey, message );
    operationSucceeded->set_value( true );
}

void RabbitConnectionHandler::doBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        DeferedResultSetter operationSucceeded ) const
{
    PRINT_DEBUG(DEBUG, "binding: " << routingKey);
    _channel->bindQueue( exchangeName, queueName, routingKey );

    auto & bindHndl = _channel->bindQueue( exchangeName, queueName, routingKey );
    bindHndl.onSuccess( [ operationSucceeded ]() {
            operationSucceeded->set_value( true );
            });
    bindHndl.onError( [ operationSucceeded ] ( const char* message ) {
            operationSucceeded->set_value( false );
            } ) ;
}

void RabbitConnectionHandler::doUnBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        DeferedResultSetter operationSucceeded ) const
{
    PRINT_DEBUG(DEBUG, "unbinding: " << routingKey);
    _channel->unbindQueue( exchangeName, queueName, routingKey );
    auto & unBindHndl = _channel->unbindQueue( exchangeName, queueName, routingKey );
    unBindHndl.onSuccess([ operationSucceeded ]() {
            operationSucceeded->set_value( true );
            });
    unBindHndl.onError( [ operationSucceeded ] ( const char* message ) {
             operationSucceeded->set_value( false );
             PRINT_DEBUG(DEBUG, "failed binding");
             } ) ;
}

void RabbitConnectionHandler::onConnected( AMQP::Connection * connection )
{
    //_connection ptr will not dangle because we reset it after disconnection, so channel is invalid anyway...
    //Nevertheless, maybe add channel ctor that takes shared_ptr.
    _channel.reset( new AMQP::Channel( _connection.get() ) ); 
    _channel->onError([ this ](const char *message) {
            PRINT_DEBUG(DEBUG, "channel error " << message);
            _connectionState.disconnected();
            });

    //TODO: 
    //this CB is called twice: 
    //from AMQP::ConnectionOpenOKFrame::process ->  
    //          AMQP::ConnectionImpl::setConnected  ->
    //               AMQP::RabbitConnectionHandler::onConnected ( this function, on exit )->
    //                   AMQP::Channel::onReady ->
    //                      AMQP::ChannelImpl::onReady
    // and: 
    //  AMQP::ChannelOpenOKFrame::process ->
    //      AMQP::ChannelImpl::reportReady 
    //          
    //This is because we install teh CB when the channel is already connected, so the FW called this CB
    //on installation.
    //This indicates that we can install the CB on the channel, before the channel is connected, 
    //which may indicate that we can:
    //1. create is in advance
    //2. reuse it on reconnection
    //3. something else we can think of.
    //At the minimum we should filter out the second call (based on _connected )
    _channel->onReady([ this ]() {
            PRINT_DEBUG(DEBUG, "channel ready ");
            _connectionState.loggedIn();
    });
}

void RabbitConnectionHandler::onData(AMQP::Connection *connection, const char *data, size_t size)
{
    _socket->send( data, size );
}

void RabbitConnectionHandler::onError(AMQP::Connection *connection, const char *message)
{
    //todo: this function is being called when we get a formal close connection from the broker or when formally closing broker.
    //todo: the consumer is unaware that he lost connectivity, but it must, so it can reconnect
    //todo: not every onError, is caused by formal disconnect... we should be aware of the difference and maybe just call _connection.close() + reconnect
    PRINT_DEBUG(DEBUG, "(onError)Error: "<< message);
    _connectionState.disconnected();
}

void RabbitConnectionHandler::onClosed(AMQP::Connection *connection) 
{
    PRINT_DEBUG(DEBUG, "Info: Connection Closed");
    _connectionState.disconnected();
}

void RabbitConnectionHandler::login( const std::string & userName,
        const std::string & password,
        DeferedResultSetter operationSucceeded )
{
    _connectionState.loggingIn( operationSucceeded );
    Login login( userName, password );
    _connection.reset( new AMQP::Connection(this, login, std::string( "/" ) ) );
}

void RabbitConnectionHandler::declareQueue( const std::string & queueName, 
        bool isDurable, 
        bool isExclusive, 
        bool isAutoDelete,
        DeferedResultSetter operationSucceeded,
        OnMessageReceivedCB onMsgReceivedCB ) const
{
    int flags = 0;
    if( isDurable )       flags |= AMQP::durable;
    // TODO: why was it commented out?!
    if( isExclusive )     flags |= AMQP::exclusive;
    if( isAutoDelete )    flags |= AMQP::autodelete; 

    AMQP::Table arguments;
//    arguments["x-dead-letter-exchange"] = "some-exchange";
    arguments["x-message-ttl"] = 30 * 1000; //time in ms before message is discarded
//    arguments["x-expires"] = 7200 * 1000; //time in ms before queue is automatically deleted if idle

    auto & queueHndl = _channel->declareQueue( queueName, flags, arguments );
    queueHndl.onSuccess([ this, queueName, operationSucceeded, onMsgReceivedCB ]() { 
            PRINT_DEBUG(DEBUG, "Queue declared successfully");
            operationSucceeded->set_value( true );
            _channel->consume( queueName.c_str() ).onReceived([ onMsgReceivedCB, this ](const AMQP::Message &message, 
                    uint64_t deliveryTag, 
                    bool redelivered ) {
                onMsgReceivedCB( message );
                _channel->ack( deliveryTag );
                } ) ;
            }); 
    queueHndl.onError( [ operationSucceeded ] ( const char* message ) {
            PRINT_DEBUG(DEBUG, "Failed declaring queue. error: " << message);
            operationSucceeded->set_value( false );
            } );
}

void RabbitConnectionHandler::declareExchange( const std::string & exchangeName,
        ExchangeType type, 
        bool isDurable,
        DeferedResultSetter operationSucceeded ) const
{
    PRINT_DEBUG(DEBUG, "declaring exchange: " << exchangeName);
    int flags = 0;
    if( isDurable )     flags |= AMQP::durable;

    auto & exchangeHndl = _channel->declareExchange( exchangeName, type, flags );
    exchangeHndl.onSuccess([ operationSucceeded ]() { 
            PRINT_DEBUG(DEBUG, "Exchange declared successfully");
            operationSucceeded->set_value( true );
            });
    exchangeHndl.onError( [ operationSucceeded ] (const char* message ) {
                operationSucceeded->set_value( false );
                PRINT_DEBUG(DEBUG, "Failed declaring exchange. error: " << message);
                } ) ;
}

} //namespace AMQP
