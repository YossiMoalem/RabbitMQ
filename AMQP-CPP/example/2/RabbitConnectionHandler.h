#ifndef AMQP_CLIENT_IMPLE_H
#define AMQP_CLIENT_IMPLE_H

#include <boost/noncopyable.hpp>
#include <memory>
#include <amqpcpp.h>

#include "Types.h"

namespace AMQP {
class RabbitSocket;
class ConnectionState;

class RabbitConnectionHandler : private AMQP::ConnectionHandler, boost::noncopyable 
{
 public:

    RabbitConnectionHandler( std::function<int( const AMQP::Message& )> onMsgReceivedCB,
            ConnectionState & connectionState,
            std::shared_ptr< RabbitSocket > socket) ;

    //TODO: Need to prevent caling, or, trying to execute those function before we have a valid channed.
    void doPublish( const std::string & exchangeName,
            const std::string & routingKey, 
            const std::string & message, 
            DeferedResultSetter operationSucceeded ) const;

    void doBindQueue( const std::string & exchangeName, 
            const std::string & queueName, 
            const std::string & routingKey,  
            DeferedResultSetter operationSucceeded ) const;

    void doUnBindQueue( const std::string & exchangeName, 
            const std::string & queueName, 
            const std::string & routingKey, 
            DeferedResultSetter operationSucceeded ) const;

    void declareExchange( const std::string & exchangeName, 
            ExchangeType type, 
            bool durable,
            DeferedResultSetter operationSucceeded ) const;

    void declareQueue( const std::string & queueName, 
            bool durable, 
            bool exclusive, 
            bool autoDelete,
            DeferedResultSetter operationSucceeded,
            OnMessageReceivedCB onMsgReceivedCB ) const;

    void removeQueue( const std::string & queueName,
            DeferedResultSetter operationSucceeded ) const;

    void login( const std::string & userName, 
            const std::string & password,
            DeferedResultSetter operationSucceeded );

    int parse( const char *  data, size_t size )
    { return _connection->parse( data, size ) ; }

    virtual void onConnected( AMQP::Connection *connection ) override ;

    virtual void onData(AMQP::Connection *connection, const char *data, size_t size) override;

    virtual void onError(AMQP::Connection *connection, const char *message) override;

    virtual void onClosed(AMQP::Connection *connection) override;

 private:
    std::shared_ptr< RabbitSocket >              _socket;
    std::unique_ptr< AMQP::Connection >          _connection { nullptr };
    std::unique_ptr< AMQP::Channel >             _channel { nullptr };
    ConnectionState &                            _connectionState;
};

} //namespace AMQP
#endif
