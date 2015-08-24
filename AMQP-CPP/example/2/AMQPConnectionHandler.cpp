#include "AMQPConnectionHandler.h"
#include "AMQPConnectionDetails.h"

#include <assert.h>
#include <memory>
#include "Debug.h"
#include "Heartbeat.h"

namespace AMQP {

AMQPConnectionHandler::AMQPConnectionHandler( std::function<int( const AMQP::Message& )> onMsgReceivedCB,
        ConnectionState & connectionState ) :
    _onMsgReceivedBC( onMsgReceivedCB ),
    _connectionState( connectionState )
{ }

AMQPConnectionHandler::~AMQPConnectionHandler()
{
    if( _connection )
        delete _connection;
    if( _channel )
        delete _channel;
}

bool AMQPConnectionHandler::handleInput( )
{
    if( _socket.read( _incomingMessages ) )
    {
        size_t processed = _connection->parse( _incomingMessages.data(), _incomingMessages.size() );
        _incomingMessages.shrink( processed );
        return true;
    }
    return false;
}

bool AMQPConnectionHandler::handleOutput()
{
    assert( pendingSend() );
    return _socket.send( _outgoingBuffer);
}

bool AMQPConnectionHandler::pendingSend()
{
    return ! _outgoingBuffer.empty();
}

void AMQPConnectionHandler::doPublish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message, 
        DeferedResultSetter operationSucceeded ) const
{
    _channel->publish( exchangeName, routingKey, message );
    operationSucceeded->set_value( true );
}

void AMQPConnectionHandler::doBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        DeferedResultSetter operationSucceeded ) const
{
    PRINT_DEBUG(DEBUG, "binding: " << routingKey);
    _channel->bindQueue( exchangeName, queueName, routingKey );

//    slow binding
//    operationSucceeded->set_value( true );
    auto & bindHndl = _channel->bindQueue( exchangeName, queueName, routingKey );
    bindHndl.onSuccess( [ exchangeName, queueName, routingKey, operationSucceeded ]() {
            operationSucceeded->set_value( true );
            });
    bindHndl.onError( [ operationSucceeded ] ( const char* message ) {
            operationSucceeded->set_value( false );
            } ) ;
}

void AMQPConnectionHandler::doUnBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        DeferedResultSetter operationSucceeded ) const
{
    PRINT_DEBUG(DEBUG, "unbinding: " << routingKey);
    _channel->unbindQueue( exchangeName, queueName, routingKey );
//    slow binding
//    operationSucceeded->set_value( true );
    auto & unBindHndl = _channel->unbindQueue( exchangeName, queueName, routingKey );
    unBindHndl.onSuccess([ operationSucceeded ]() {
            operationSucceeded->set_value( true );
            });
    unBindHndl.onError( [ operationSucceeded ] ( const char* message ) {
             operationSucceeded->set_value( false );
             PRINT_DEBUG(DEBUG, "failed binding");
             } ) ;
}

void AMQPConnectionHandler::onConnected( AMQP::Connection * connection )
{
    if( _channel )
        delete _channel;
    _channel = new AMQP::Channel( _connection );

    _channel->onError([ this ](const char *message) {
            PRINT_DEBUG(DEBUG, "channel error " << message);
            });

    //TODO: 
    //this CB is called twice: 
    //from AMQP::ConnectionOpenOKFrame::process ->  
    //          AMQP::ConnectionImpl::setConnected  ->
    //               AMQP::AMQPConnectionHandler::onConnected ( this function, on exit )->
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

void AMQPConnectionHandler::onData(AMQP::Connection *connection, const char *data, size_t size)
{
    _outgoingBuffer.append( data, size );
}

void AMQPConnectionHandler::onError(AMQP::Connection *connection, const char *message)
{
    //todo: this function is being called when we get a formal close connection from the broker or when formally closing broker.
    //todo: the consumer is unaware that he lost connectivity, but it must, so it can reconnect
    //todo: not every onError, is caused by formal disconnect... we should be aware of the difference and maybe just call _connection.close() + reconnect
    PRINT_DEBUG(DEBUG, "(onError)Error: "<< message);
    _connectionState.disconnected();
}

void AMQPConnectionHandler::onClosed(AMQP::Connection *connection) 
{
    PRINT_DEBUG(DEBUG, "Info: Connection Closed");
    _connectionState.disconnected();
}

void AMQPConnectionHandler::login( const std::string & userName,
        const std::string & password,
        DeferedResultSetter operationSucceeded )
{
    _connectionState.loggingIn( operationSucceeded );
    Login login( userName, password );
    _connection = new AMQP::Connection(this, login, std::string( "/" ) );
}

void AMQPConnectionHandler::declareQueue( const std::string & queueName, 
        bool isDurable, 
        bool isExclusive, 
        bool isAutoDelete,
        DeferedResultSetter operationSucceeded ) const
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
    queueHndl.onSuccess([ this, queueName, operationSucceeded ]() { 
            PRINT_DEBUG(DEBUG, "Queue declared successfully");
            operationSucceeded->set_value( true );
            _channel->consume( queueName.c_str() ).onReceived([ this ](const AMQP::Message &message, 
                    uint64_t deliveryTag, 
                    bool redelivered ) {
                if( _onMsgReceivedBC )
                {
                    _onMsgReceivedBC( message );
                }
                _channel->ack( deliveryTag );
                } ) ;
            }); 
    queueHndl.onError( [ operationSucceeded ] ( const char* message ) {
            PRINT_DEBUG(DEBUG, "Failed declaring queue. error: " << message);
            operationSucceeded->set_value( false );
            } );
}

void AMQPConnectionHandler::declareExchange( const std::string & exchangeName,
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

bool AMQPConnectionHandler::connect(const AMQPConnectionDetails & connectionParams )
{
    _connectionState.socketConnecting();
    _incomingMessages.clear();
    _outgoingBuffer.clear();
    PRINT_DEBUG(DEBUG, "Connecting to : "<<connectionParams._host<<":"<<connectionParams._port);
    if( ! _socket.connect( connectionParams._host, connectionParams._port ) )
    {
        _connectionState.disconnected();
        PRINT_DEBUG(DEBUG, "Error creating socket");
        return false;
    } else {
        _connectionState.socketConnected();
        return true;
    }
}

void AMQPConnectionHandler::closeSocket()
{
    _socket.close();
}

unsigned int AMQPConnectionHandler::outgoingBufferSize() const
{
    return _outgoingBuffer.size();
}

} //namespace AMQP
