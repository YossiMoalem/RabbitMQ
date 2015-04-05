#include "AMQPConnectionHandler.h"
#include "AmqpConnectionDetails.h"

namespace AMQP {

AMQPConnectionHandler::AMQPConnectionHandler( std::function<int( const AMQP::Message& )> onMsgReceivedCB ) :
    _onMsgReceivedBC( onMsgReceivedCB )
{}

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
    }
    return true;
}

bool AMQPConnectionHandler::handleOutput()
{
    if( ! _outgoingMessages.empty() )
    {
        return _socket.send( _outgoingMessages );
    }
    return false;
}

bool AMQPConnectionHandler::pendingSend()
{
    return ! _outgoingMessages.empty();
}

void AMQPConnectionHandler::doPublish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message, 
        RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const
{
    _channel->publish( exchangeName, routingKey, message );
    operationSucceeded->set_value( true );
}

void AMQPConnectionHandler::doBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const
{
    auto & bindHndl = _channel->bindQueue( exchangeName, queueName, routingKey );
    bindHndl.onSuccess([ exchangeName, queueName, routingKey, operationSucceeded ]() {
            operationSucceeded->set_value( true );
            });
    bindHndl.onError( [ operationSucceeded ] ( const char* message ) {
            operationSucceeded->set_value( false );
            } ) ;
}

void AMQPConnectionHandler::doUnBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const
{
    auto & unBindHndl = _channel->unbindQueue( exchangeName, queueName, routingKey );
    unBindHndl.onSuccess([ operationSucceeded ]() {
            operationSucceeded->set_value( true );
            });
    unBindHndl.onError( [ operationSucceeded ] ( const char* message ) {
             operationSucceeded->set_value( false );
             std::cout <<"failed binding" <<std::endl;
             } ) ;
}

void AMQPConnectionHandler::onConnected( AMQP::Connection *connection )
{
    if( _channel )
        delete _channel;
    _channel = new AMQP::Channel(_connection);

    _channel->onError([](const char *message) {
            std::cout << "channel error " << message << std::endl;
            });

    _channel->onReady([ this ]() {
            _connected = true;
            });
}

void AMQPConnectionHandler::onData(AMQP::Connection *connection, const char *data, size_t size)
{
    _outgoingMessages.append( data, size );
    _socket.send( _outgoingMessages );
}

void AMQPConnectionHandler::onError(AMQP::Connection *connection, const char *message)
{
    std::cout <<"Error: Error: "<< message <<std::endl;
}

void AMQPConnectionHandler::onClosed(AMQP::Connection *connection) 
{
    std::cout <<"Info: Connection Closed"<< std::endl;
}

bool AMQPConnectionHandler::login( const AmqpConnectionDetails & connectionParams )
{
    if( ! _socket.connect( connectionParams._host, connectionParams._port ) )
    {
        std::cout <<"Error creating socket" <<std::endl;
    } else {
        Login login( connectionParams._userName, connectionParams._password );
        _connection = new AMQP::Connection(this, login, std::string( "/" ) );

        while( !_connected )
        {
            handleInput( );
        }
    }
    return _connected;
}
std::future< bool > AMQPConnectionHandler::declareQueue( const std::string & queueName, 
        bool durable, 
        bool exclusive, 
        bool autoDelete ) const
{
    RabbitMessageBase::OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    if( !_connected )
    {
        std::cout <<"ERROR!!" <<std::endl;
    }
    int flags = 0;
    if( durable )       flags |= AMQP::durable;
    if( exclusive )     flags |= exclusive;
    if( autoDelete )    flags |= autoDelete; 

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
            std::cout <<"queue decleration faled " <<std::endl;
            } );
    return operationSucceeded->get_future();
}

std::future< bool > AMQPConnectionHandler::declareExchange( const std::string & exchangeName,
        ExchangeType type, 
        bool durable ) const
{
    RabbitMessageBase::OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    if( !_connected )
    {
        std::cout <<"ERROR!!" <<std::endl;
    }
    int flags = 0;
    if( durable )
        flags |= durable;

    auto & exchangeHndl = _channel->declareExchange( exchangeName, type, flags );
    exchangeHndl.onSuccess([ operationSucceeded ]() { 
            operationSucceeded->set_value( true );
            });
    exchangeHndl.onError( [ operationSucceeded ] (const char* message ) {
                operationSucceeded->set_value( false );
                std::cout<<"Error Declaring Exchange" << std::endl;
                } ) ;
    return operationSucceeded->get_future();
}

int AMQPConnectionHandler::getReadFD() const
{
    return _socket.readFD();
}

} //namespace AMQP

