#include "myConnectionHandler.h"
#include "AmqpConnectionDetails.h"
#include "RabbitMessage.h"

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
    _channel = new AMQP::Channel(_connection);

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
    return operationSucceeded->get_future();
}

MyConnectionHandler::OperationSucceeded MyConnectionHandler::bindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey)
{
    OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    BindMessage * bindMessage = new BindMessage( exchangeName, queueName, routingKey, operationSucceeded );
    _jobQueue.push( bindMessage );
    return operationSucceeded->get_future();
}

void MyConnectionHandler::doBindQueue( const std::string & exchangeName, 
        const std::string & queueName, 
        const std::string & routingKey, 
        OperationSucceededSetter operationSucceeded )
{
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

MyConnectionHandler::OperationSucceeded MyConnectionHandler::unBindQueue( const std::string & exchangeName, const std::string & queueName, const std::string & routingKey)
{
    OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    UnBindMessage * unBindMessage = new UnBindMessage( exchangeName, queueName, routingKey, operationSucceeded );
    _jobQueue.push( unBindMessage );
    return operationSucceeded->get_future();
}

void MyConnectionHandler::doUnBindQueue( const std::string & exchangeName, const std::string & queueName, const std::string & routingKey, OperationSucceededSetter operationSucceeded )
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

MyConnectionHandler::OperationSucceeded MyConnectionHandler::publish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message )
{
    OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    PostMessage * msg = new PostMessage( exchangeName, routingKey, message, operationSucceeded );

    _jobQueue.push( msg );
    return operationSucceeded->get_future();
}
void MyConnectionHandler::doPublish( const std::string & exchangeName, 
        const std::string & routingKey, 
        const std::string & message, 
        OperationSucceededSetter operationSucceeded )
{
    //std::cout <<"publishing: "<<message <<" to: " << routingKey << " via: " << exchangeName << std::endl;
    _channel->publish( exchangeName, routingKey, message );
    operationSucceeded->set_value( true );
}

int MyConnectionHandler::startEventLoop()
{
    if (!_connected )
        return 1;

    while( true )
    {
        handleResponse();
        handleQueue();
    }
    return 0;
}

void MyConnectionHandler::handleQueue( )
{
    RabbitMessageBase * msg = nullptr;
    if( _jobQueue.try_pop( msg ) )
    {
        switch( msg->messageType() )
        {
            case MessageType::Post:
                {
                    PostMessage * postMessage = static_cast< PostMessage* >( msg );
                    doPublish( postMessage->exchangeName(), 
                            postMessage->routingKey(), 
                            postMessage->message(), 
                            postMessage->resultSetter() );
                    delete msg;
                }
                break;
            case MessageType::Bind:
                {
                    BindMessage * bindMessage = static_cast< BindMessage* >( msg );
                    doBindQueue(bindMessage->exchangeName(), 
                            bindMessage->queueName(), 
                            bindMessage->routingKey(), 
                            bindMessage->resultSetter() ); 
                }
                break;
            case MessageType::UnBind:
                {
                    UnBindMessage * unBindMessage = static_cast< UnBindMessage* >( msg );
                    doUnBindQueue(unBindMessage->exchangeName(), 
                            unBindMessage->queueName(), 
                            unBindMessage->routingKey(), 
                            unBindMessage->resultSetter() ); 
                }
                break;
        }
    }
}
void MyConnectionHandler::handleResponse( )
{
    if( _socket.read( _sb ) )
    {
        size_t processed = _connection->parse( _sb.data(), _sb.size() );
        if( _sb.size() - processed != 0 )
        {
            std::cout <<"Ulala! got "<<_sb.size()<<" bytes, parsed: "<<processed<<"only "<<std::endl;
        }
        _sb.shrink( processed );
    }
}

bool MyConnectionHandler::connected() const
{ 
    return _connected;
}
} //namespace AMQP
