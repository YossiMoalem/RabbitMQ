#include "AMQPConnectionHandler.h"
#include "AMQPConnectionDetails.h"

#include <assert.h>
#include <memory>

#include "Heartbeat.h"

//TODO: needs to be removed!
#include "AMQPEventLoop.h"
namespace AMQP {

AMQPConnectionHandler::AMQPConnectionHandler( std::function<int( const AMQP::Message& )> onMsgReceivedCB, AMQPEventLoop * eventLoop  ) :
    _onMsgReceivedBC( onMsgReceivedCB ),
    _heartbeat( new Heartbeat( this ) ),
    _eventLoop( eventLoop )
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
    if( ! _outgoingBuffer.empty() )
    {
        return _socket.send( _outgoingBuffer);
    }
    return false;
}

bool AMQPConnectionHandler::pendingSend()
{
    return ! _outgoingBuffer.empty();
}

void AMQPConnectionHandler::doPublish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message, 
        RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    _channel->publish( exchangeName, routingKey, message );
    operationSucceeded->set_value( true );
}

void AMQPConnectionHandler::doBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    std::cout << "binding: " << routingKey << std::endl;
    _channel->bindQueue( exchangeName, queueName, routingKey );
    operationSucceeded->set_value( true );
//    auto & bindHndl = _channel->bindQueue( exchangeName, queueName, routingKey );
//    bindHndl.onSuccess( [ exchangeName, queueName, routingKey, operationSucceeded ]() {
//            operationSucceeded->set_value( true );
//            });
//    bindHndl.onError( [ operationSucceeded ] ( const char* message ) {
//            operationSucceeded->set_value( false );
//            } ) ;
}

void AMQPConnectionHandler::doUnBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    std::cout << "unbinding: " << routingKey << std::endl;
    _channel->unbindQueue( exchangeName, queueName, routingKey );
    operationSucceeded->set_value( true );
//    auto & unBindHndl = _channel->unbindQueue( exchangeName, queueName, routingKey );
//    unBindHndl.onSuccess([ operationSucceeded ]() {
//            operationSucceeded->set_value( true );
//            });
//    unBindHndl.onError( [ operationSucceeded ] ( const char* message ) {
//             operationSucceeded->set_value( false );
//             std::cout <<"failed binding" <<std::endl;
//             } ) ;
}

void AMQPConnectionHandler::onConnected( AMQP::Connection * connection )
{
    if( _channel )
        delete _channel;
    _channel = new AMQP::Channel( _connection );

    _channel->onError([](const char *message) {
            std::cout << "channel error " << message << std::endl;
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
            std::cout <<"channel ready "<<std::endl;
            _connected = true;
    });
    if( _loginValueSetter )
    {
        //TODO: detect failure and populate with false!
        //TODO: if EL stops before login finished - populate eit false
        _loginValueSetter->set_value( true );
        _loginValueSetter = nullptr;
    }
    _heartbeat->initialize();
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
    std::cout <<"(onError)Error: "<< message <<std::endl;
    closeSocket();
}

void AMQPConnectionHandler::onClosed(AMQP::Connection *connection) 
{
    std::cout <<"Info: Connection Closed"<< std::endl;
}

void AMQPConnectionHandler::login( const std::string & userName,
        const std::string & password,
        RabbitMessageBase::DeferedResultSetter operationSucceeded )
{
    Login login( userName, password );
    _connection = new AMQP::Connection(this, login, std::string( "/" ) );
    _loginValueSetter = operationSucceeded;
}

//void AMQPConnectionHandler::handleTimeout( const std::string & exchangeName) const
bool AMQPConnectionHandler::handleTimeout() const
{
    if( _connected )
    {
        return _heartbeat->send();
    } else {
        std::cout <<"TO after we are NOT connected. Ignoring " <<std::endl;
        return false;
    }

}

void AMQPConnectionHandler::declareQueue( const std::string & queueName, 
        bool isDurable, 
        bool isExclusive, 
        bool isAutoDelete,
        RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    int flags = 0;
    if( isDurable )       flags |= AMQP::durable;
    // TODO: why was it commented out?!
    if( isExclusive )     flags |= AMQP::exclusive;
    if( isAutoDelete )    flags |= AMQP::autodelete; 

    auto & queueHndl = _channel->declareQueue( queueName, flags );
    queueHndl.onSuccess([ this, queueName, operationSucceeded ]() { 
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
            operationSucceeded->set_value( false );
            std::cout <<"Failed declaring queue. error: " << message << std::endl;
            } );
}

void AMQPConnectionHandler::declareExchange( const std::string & exchangeName,
        ExchangeType type, 
        bool isDurable,
        RabbitMessageBase::DeferedResultSetter operationSucceeded ) const
{
    std::cout <<"declaring exchange: " << exchangeName <<std::endl;
    int flags = 0;
    if( isDurable )     flags |= AMQP::durable;

    auto & exchangeHndl = _channel->declareExchange( exchangeName, type, flags );
    exchangeHndl.onSuccess([ operationSucceeded ]() { 
            std::cout <<"Exchange declared successfully" <<std::endl;
            operationSucceeded->set_value( true );
            });
    exchangeHndl.onError( [ operationSucceeded ] (const char* message ) {
                operationSucceeded->set_value( false );
                std::cout<<"Failed declaring exchange. error: " << message << std::endl;
                } ) ;
}

int AMQPConnectionHandler::getReadFD() const
{
    return _socket.readFD();
}

int AMQPConnectionHandler::getWriteFD() const
{
    return _socket.readFD();
}

bool AMQPConnectionHandler::openConnection(const AMQPConnectionDetails & connectionParams )
{
    assert (! _connected );
    _incomingMessages.clear();
    _outgoingBuffer.clear();
    if( ! _socket.connect( connectionParams._host, connectionParams._port ) )
    {
        std::cout <<"Error creating socket" <<std::endl;
        return false;
    } else {
        return true;
    }
}
void AMQPConnectionHandler::closeSocket()
{
    _connected = false;
    _heartbeat->invalidate();
    _socket.close();
}
} //namespace AMQP

