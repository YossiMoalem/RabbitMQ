#include "AMQPConnection.h"
#include "AmqpConnectionDetails.h"

namespace AMQP {
AMQPConnection::AMQPConnection( OnMessageReveivedCB onMsgReceivedCB ) :
    _onMsgReceivedBC( onMsgReceivedCB )
{}

AMQPConnection::~AMQPConnection()
{
    if( _connection )
        delete _connection;
    if( _channel )
        delete _channel;
}

void AMQPConnection::doPublish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message, 
        OperationSucceededSetter operationSucceeded ) const
{
    //std::cout <<"L2:publishing: "<<message <<" to: " << routingKey << " via: " << exchangeName << std::endl;
    _channel->publish( exchangeName, routingKey, message );
    operationSucceeded->set_value( true );
}

void AMQPConnection::doBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        OperationSucceededSetter operationSucceeded ) const
{
    std::cout <<"L2:Going to bind: "<<exchangeName <<" : " << queueName << " to : " << routingKey <<std::endl;
    auto & bindHndl = _channel->bindQueue( exchangeName, queueName, routingKey );
    bindHndl.onSuccess([ exchangeName, queueName, routingKey, operationSucceeded ]() {
            std::cout << "*** queue "<< queueName <<" bound to exchange " <<exchangeName <<" on: " << routingKey << std::endl;
            operationSucceeded->set_value( true );
            });
    bindHndl.onError( [ operationSucceeded ] ( const char* message ) {
            operationSucceeded->set_value( false );
            std::cout <<"failed binding" <<std::endl;
            } ) ;
}

void AMQPConnection::doUnBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        OperationSucceededSetter operationSucceeded ) const
{
    auto & unBindHndl = _channel->unbindQueue( exchangeName, queueName, routingKey );
    unBindHndl.onSuccess([ operationSucceeded ]() {
            std::cout << "queue bound to exchange" << std::endl;
            operationSucceeded->set_value( true );
            });
    unBindHndl.onError( [ operationSucceeded ] ( const char* message ) {
             operationSucceeded->set_value( false );
             std::cout <<"failed binding" <<std::endl;
             } ) ;
}

void AMQPConnection::handleInput( )
{
    if( _socket.read( _incomingMessages ) )
    {
        size_t processed = _connection->parse( _incomingMessages.data(), _incomingMessages.size() );
        if( _incomingMessages.size() - processed != 0 )
        {
            std::cout <<"Ulala! got "<<_incomingMessages.size()<<" bytes, parsed: "<<processed<<"only "<<std::endl;
        }
        _incomingMessages.shrink( processed );
    }
}
void AMQPConnection::onConnected( AMQP::Connection *connection )
{
    std::cout << "AMQP login success" << std::endl;

    if( _channel )
        delete _channel;
    _channel = new AMQP::Channel(_connection);

    _channel->onError([](const char *message) {
            std::cout << "channel error " << message << std::endl;
            });

    _channel->onReady([ this ]() {
            std::cout << "channel ready" << std::endl;
            _connected = true;
            });
}

void AMQPConnection::onData(AMQP::Connection *connection, const char *data, size_t size)
{
    _outgoingMessages.append( data, size );
    _socket.send( _outgoingMessages );
}

void AMQPConnection::onError(AMQP::Connection *connection, const char *message)
{
    std::cout <<"Error: Error: "<< message <<std::endl;
}

void AMQPConnection::onClosed(AMQP::Connection *connection) 
{
    std::cout <<"Info: Connection Closed"<< std::endl;
}

bool AMQPConnection::login( const AmqpConnectionDetails & connectionParams )
{
    if( ! _socket.connect( connectionParams._host, connectionParams._port ) )
    {
        std::cout <<"Error creating socket" <<std::endl;
    } else {
        std::cout << "connected" << std::endl;
        Login login( connectionParams._userName, connectionParams._password );
        _connection = new AMQP::Connection(this, login, std::string( "/" ) );

        while( !_connected )
        {
            handleInput( );
        }
    }
    return _connected;
}
std::future< bool > AMQPConnection::declareQueue( const std::string & queueName, 
        bool durable, 
        bool exclusive, 
        bool autoDelete ) const
{
   OperationSucceededSetter operationSucceeded( new std::promise< bool > );
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
            std::cout << "queue declared" << std::endl; 
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

std::future< bool > AMQPConnection::declareExchange( const std::string & exchangeName,
        ExchangeType type, 
        bool durable ) const
{
   OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    if( !_connected )
    {
        std::cout <<"ERROR!!" <<std::endl;
    }
    int flags = 0;
    if( durable )
        flags |= durable;

    auto & exchangeHndl = _channel->declareExchange( exchangeName, type, flags );
    exchangeHndl.onSuccess([ operationSucceeded ]() { 
            std::cout << "exchange declared" << std::endl; 
            operationSucceeded->set_value( true );
            });
    exchangeHndl.onError( [ operationSucceeded ] (const char* message ) {
                operationSucceeded->set_value( false );
                std::cout<<"Error Declaring Exchange" << std::endl;
                } ) ;
    return operationSucceeded->get_future();
}

} //namespace AMQP

