#include "myConnectionHandler.h"
#include "AmqpConnectionDetails.h"
#include <amqpcpp.h>

#include <iostream> 
#include <memory>

namespace AMQP {

MyConnectionHandler::MyConnectionHandler( CB onMsgReceivedCB ) :
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
    _connected = true;
    std::cout << "AMQP login success" << std::endl;

    if( _channel )
        delete _channel;
    _channel = new AMQP::Channel(_connection);
    handleResponse( ); //AMQP::ChannelOpenOKFrame::process

    // install a handler when channel is in error
    _channel->onError([](const char *message) {
            std::cout << "channel error " << message << std::endl;
            });

    // install a handler when channel is ready
    _channel->onReady([ this ]() {
            std::cout << "channel ready" << std::endl;
            _channelReady = true;
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
        // create amqp connection, and a new channel 
        // Sends protocol header: "AMQP\0[majorVer][minorVer][rev]
        _connection = new AMQP::Connection(this, AMQP::Login( connectionParams._userName, connectionParams._password ), std::string( "/" ) );

        while( !_connected )
        {
            //Expect to get: 
            // *ConnectionStartFrame: validate protocol and send properpeis and credential
            // *ConnectionTuneFrame send: 
            //      **ConnectionTuneOKFrame(channelMax(), frameMax(), heartbeat()), and
            //      **connectionOpenFrame(connection->vhost())
            // *ConnectionOpenOKFrame:  doing :AMQP::ConnectionImpl::setConnected(), calling onConnected
            handleResponse( );
        }
    }
    return _connected;
}

void MyConnectionHandler::declareQueue( const char * queueName )
{
    if( !_channelReady )
    {
        std::cout <<"ERROR!!" <<std::endl;
    }
    _queueName = std::string( queueName );
    _channel->declareQueue( queueName ).onSuccess([this]() { 
            std::cout << "queue declared" << std::endl; 
            // start consuming
            _channel->consume( _queueName.c_str() ).onReceived([ this ](const AMQP::Message &message, 
                    uint64_t deliveryTag, 
                    bool redelivered) {
               // std::cout << "received: " << message.message() << std::endl;
                _channel->ack( deliveryTag );
                if( _onMsgReceivedBC )
                    _onMsgReceivedBC( message );
                });
            });
    handleResponse( ); //AMQP::QueueDeclareOKFrame::QueueDeclareOKFrame
    handleResponse( );//AMQP::BasicConsumeOKFrame::BasicConsumeOKFrame
}

void MyConnectionHandler::declareExchange( const std::string & exchangeName, ExchangeType type, bool durable )
{
    if( !_channelReady )
    {
        std::cout <<"ERROR!!" <<std::endl;
    }
    int flags = 0;
    if( durable )
        flags |= durable;

    _channel->declareExchange( exchangeName, type, flags ).onSuccess([]() { 
            std::cout << "exchange declared" << std::endl; 
            });
    handleResponse(); //AMQP::ExchangeDeclareOKFrame::ExchangeDeclareOKFrame
}

void MyConnectionHandler::bindQueue( const std::string & exchangeName, const std::string & queueName, const std::string & routingKey)
{
    _channel->bindQueue( exchangeName, queueName, routingKey ).onSuccess([ this, exchangeName ]() {
            std::cout << "*** queue "<< _queueName <<" bound to exchange " <<exchangeName <<" on: " << _routingKey << std::endl;
            });
    handleResponse( ); //AMQP::QueueBindOKFrame::QueueBindOKFrame
}

void MyConnectionHandler::unbindQueue( const std::string & exchangeName, const std::string & queueName, const std::string & routingKey)
{
    _channel->unbindQueue( exchangeName, queueName, routingKey ).onSuccess([this]() {
            std::cout << "queue bound to exchange" << std::endl;
            });
    handleResponse( ); //AMQP::QueueBindOKFrame::QueueBindOKFrame
}

void MyConnectionHandler::receiveMessage()
{
    handleResponse();
}

void MyConnectionHandler::publish( const std::string & exchangeName, const char* routingKey, const char* message )
{
    //std::cout <<"publishing: "<<message <<" to: " << routingKey << " via: " << exchangeName << std::endl;
    _channel->publish( exchangeName.c_str(), routingKey, message );
}

void MyConnectionHandler::handleResponse( )
{
    const int buffsize = 1024;
    char buff[buffsize];
    size_t size = _socket.read( buff, buffsize );
    size_t processed = _connection->parse( buff, size ); // TODO: check how much we processes and keep the rest!:
    if( size - processed != 0 )
    {
        std::cout <<"Ulala! got "<<size<<" bytes, parsed: "<<processed<<"only "<<std::endl;
    }
}
} //namespace AMQP
