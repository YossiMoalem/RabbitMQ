#include "myConnectionHandler.h"
#include "AmqpConnectionDetails.h"

#include <iostream> 

namespace AMQP {

MyConnectionHandler::MyConnectionHandler( OnMessageReveivedCB onMsgReceivedCB ) :
    _connection( nullptr ),
    _channel( nullptr ),
    _onMsgReceivedBC( onMsgReceivedCB )
{ }

MyConnectionHandler::~MyConnectionHandler()
{
    if( _connection )
        delete _connection;
    if( _channel )
        delete _channel;
}

void MyConnectionHandler::onConnected( AMQP::Connection *connection )
{
    std::cout << "AMQP login success" << std::endl;

    if( _channel )
        delete _channel;
    _sb.clear();
    _channel = new AMQP::Channel(_connection);
    handleResponse( ); 

    _channel->onError([](const char *message) {
            std::cout << "channel error " << message << std::endl;
            });

    _channel->onReady([ this ]() {
            std::cout << "channel ready" << std::endl;
            _connected = true;
            });
}

void MyConnectionHandler::onData(AMQP::Connection *connection, const char *data, size_t size)
{
    _socket.send( data, size );
}

void MyConnectionHandler::onError(AMQP::Connection *connection, const char *message)
{
    std::cout <<"Error: Error: "<< message <<std::endl;
}

void MyConnectionHandler::onClosed(AMQP::Connection *connection) 
{
    std::cout <<"Info: Connection Closed"<< std::endl;
}

bool MyConnectionHandler::login( const AmqpConnectionDetails & connectionParams )
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
            handleResponse( );
        }
    }
    return _connected;
}

MyConnectionHandler::OperationSucceeded MyConnectionHandler::declareQueue( const std::string & queueName, bool durable, bool exclusive, bool autoDelete )
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
    handleResponse( ); //AMQP::QueueDeclareOKFrame::QueueDeclareOKFrame
    handleResponse( );//AMQP::BasicConsumeOKFrame::BasicConsumeOKFrame
    return operationSucceeded->get_future();
}

MyConnectionHandler::OperationSucceeded MyConnectionHandler::declareExchange( const std::string & exchangeName, ExchangeType type, bool durable )
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
    handleResponse(); //AMQP::ExchangeDeclareOKFrame::ExchangeDeclareOKFrame
    return operationSucceeded->get_future();
}

MyConnectionHandler::OperationSucceeded MyConnectionHandler::bindQueue( const std::string & exchangeName, const std::string & queueName, const std::string & routingKey)
{
    OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    auto & bindHndl = _channel->bindQueue( exchangeName, queueName, routingKey );
    bindHndl.onSuccess([ exchangeName, queueName, routingKey, operationSucceeded ]() {
            std::cout << "*** queue "<< queueName <<" bound to exchange " <<exchangeName <<" on: " << routingKey << std::endl;
            operationSucceeded->set_value( true );
            });
    bindHndl.onError( [ operationSucceeded ] ( const char* message ) {
            operationSucceeded->set_value( false );
            std::cout <<"failed binding" <<std::endl;
            } ) ;
    handleResponse( ); //AMQP::QueueBindOKFrame::QueueBindOKFrame
    return operationSucceeded->get_future();
}

MyConnectionHandler::OperationSucceeded MyConnectionHandler::unbindQueue( const std::string & exchangeName, const std::string & queueName, const std::string & routingKey)
{
    OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    auto & unbindHndl = _channel->unbindQueue( exchangeName, queueName, routingKey );
    unbindHndl.onSuccess([ operationSucceeded ]() {
            std::cout << "queue bound to exchange" << std::endl;
            operationSucceeded->set_value( true );
            });
    unbindHndl.onError( [ operationSucceeded ] ( const char* message ) {
             operationSucceeded->set_value( false );
             std::cout <<"failed binding" <<std::endl;
             } ) ;
    handleResponse( ); //AMQP::QueueBindOKFrame::QueueBindOKFrame
    return operationSucceeded->get_future();
}

void MyConnectionHandler::receiveMessage()
{
    handleResponse();
}

void MyConnectionHandler::publish( const std::string & exchangeName, const std::string & routingKey, const std::string & message )
{
    //std::cout <<"publishing: "<<message <<" to: " << routingKey << " via: " << exchangeName << std::endl;
    _channel->publish( exchangeName, routingKey, message );
}

void MyConnectionHandler::handleResponse( )
{
    _socket.read( _sb );
    std::cout << "going to ask to process " << _sb.size() <<std::endl;
    size_t processed = _connection->parse( _sb.data(), _sb.size() );
    if( _sb.size() - processed != 0 )
    {
        std::cout <<"Ulala! got "<<_sb.size()<<" bytes, parsed: "<<processed<<"only "<<std::endl;
    }
    _sb.shrink( processed );
    std::cout <<"after shrinking size is " << _sb.size() <<std::endl;
}

bool MyConnectionHandler::connected() const
{ 
    return _connected;
}
} //namespace AMQP
