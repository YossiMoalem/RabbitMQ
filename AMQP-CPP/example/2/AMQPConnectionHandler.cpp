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
{
    _connectionEstablishedMutex.lock();
}

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
        RabbitMessageBase::OperationSucceededSetter operationSucceeded ) const
{
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

    _channel->onReady([ this ]() {
            std::cout <<"channel ready "<<std::endl;
            _connected = true;
    });
}

void AMQPConnectionHandler::onData(AMQP::Connection *connection, const char *data, size_t size)
{
    //TODO: the buffer should implement eventFD, to signal that it has value
    //this hsould be registered by the eventloop
    _outgoingMessages.append( data, size );
    //handleOutput();
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

bool AMQPConnectionHandler::login( const AMQPConnectionDetails & connectionParams )
{
    assert (! _connected );
    _incomingMessages.clear();
    _outgoingMessages.clear();
    if( ! _socket.connect( connectionParams._host, connectionParams._port ) )
    {
        std::cout <<"Error creating socket" <<std::endl;
        _connectionEstablishedMutex.unlock();
        return false;
    } else {
        std::cout <<"socket Created!" << std::endl;
        _connectionEstablishedMutex.unlock();
        Login login( connectionParams._userName, connectionParams._password );
        _connection = new AMQP::Connection(this, login, std::string( "/" ) );

        //TODO: 
        //When this code will move to the eventloop thread, it will not be imortant 
        //but till then....
        while( !_connected  && _eventLoop->active() )
        {
            //TODO: REALLY????
            //1. create promiss
            //2. onConnected will populate it
            //3. wait on the future.
            //4. kame sure event loop was not stoped...
            sleep(1);
        }
        if ( _eventLoop->active() )
            std::cout <<"Login Done!" << std::endl;
        else
            std::cout<< "login out because _stop event loop is called";
    }
    _heartbeat->initialize();
    return _connected;
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

std::future< bool > AMQPConnectionHandler::declareQueue( const std::string & queueName, 
        bool isDurable, 
        bool isExclusive, 
        bool isAutoDelete ) const
{
    RabbitMessageBase::OperationSucceededSetter operationSucceeded( new std::promise< bool > );
    int flags = 0;
    if( isDurable )       flags |= AMQP::durable;
    //if( isExclusive )     flags |= AMQP::exclusive;
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
    return operationSucceeded->get_future();
}

std::future< bool > AMQPConnectionHandler::declareExchange( const std::string & exchangeName,
        ExchangeType type, 
        bool isDurable ) const
{
    std::cout <<"declaring exchange: " << exchangeName <<std::endl;
    RabbitMessageBase::OperationSucceededSetter operationSucceeded( new std::promise< bool > );
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
    return operationSucceeded->get_future();
}

int AMQPConnectionHandler::getReadFD() const
{
    return _socket.readFD();
}

int AMQPConnectionHandler::getOutgoingMessagesFD() const
{
    return _outgoingMessages.getFD();
}

void AMQPConnectionHandler::waitForConnection()
{
    _connectionEstablishedMutex.lock();
    _connectionEstablishedMutex.unlock();

}

void AMQPConnectionHandler::closeSocket()
{
    _connectionEstablishedMutex.try_lock();
    _connected = false;
    _heartbeat->invalidate();
    // TODO: _socket.close();
}
} //namespace AMQP

