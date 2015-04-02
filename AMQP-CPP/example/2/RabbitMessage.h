#ifndef RABBIT_MESSAGE_H
#define RABBIT_MESSAGE_H

#include <iostream>
#include <sstream>

//For OperationSucceededSetter.
//should be removed!
#include "AMQPConnection.h"

namespace AMQP {
enum class MessageType
{
    Post,
    Bind,
    UnBind
};


/********************************************************************************\
 * RabbitMessageBase
 ********************************************************************************/
class RabbitMessageBase
{
    friend std::ostream& operator<< (std::ostream& ostream, const RabbitMessageBase& inst );
 public:
    RabbitMessageBase( AMQPConnection::OperationSucceededSetter returnValueSetter ):
        _returnValueSetter( returnValueSetter )
    {}
    
    virtual ~RabbitMessageBase () {}

    RabbitMessageBase () {}
    RabbitMessageBase( const RabbitMessageBase& ) = delete;
    RabbitMessageBase& operator= (const RabbitMessageBase& ) = delete;

    AMQPConnection::OperationSucceededSetter resultSetter()
    {
        return _returnValueSetter;
    }

    virtual MessageType messageType() const = 0;

 protected:
    AMQPConnection::OperationSucceededSetter _returnValueSetter;

};

/********************************************************************************\
 * PostMessage
 ********************************************************************************/
class PostMessage : public RabbitMessageBase
{
    friend std::ostream& operator<< (std::ostream& ostream, const RabbitMessageBase& inst );
 public:
    PostMessage( const std::string & exchangeName, 
            const std::string & routingKey, 
            const std::string & message,
             AMQPConnection::OperationSucceededSetter operationSucceeded ) :
        RabbitMessageBase( operationSucceeded ),
        _exchangeName( exchangeName ),
        _routingKey( routingKey ),
        _message( message )
    {}

    MessageType messageType() const
    {
        return MessageType::Post;
    }

    const std::string & message() const
    {
        return _message;
    }

    const std::string & routingKey() const
    {
        return _routingKey;
    }

    const std::string & exchangeName() const
    {
        return _exchangeName;
    }


 protected:
    std::string _exchangeName;
    std::string _routingKey;
    std::string _message;
};

/********************************************************************************\
 * BindMessage
 ********************************************************************************/
class BindMessage : public RabbitMessageBase
{
    friend std::ostream& operator<< (std::ostream& ostream, const RabbitMessageBase& inst );
 public:
    BindMessage( const std::string & exchangeName, 
            const std::string & queueName,
            const std::string routingKey,
            AMQPConnection::OperationSucceededSetter operationSucceeded ) :
        RabbitMessageBase( operationSucceeded ),
        _exchangeName( exchangeName ),
        _queueName( queueName ),
        _routingKey( routingKey )
    { }

    MessageType messageType() const
    {
        return MessageType::Bind;
    }

    const std::string & routingKey() const
    {
        return _routingKey;
    }

    const std::string & exchangeName() const
    {
        return _exchangeName;
    }

    const std::string & queueName()
    {
        return _queueName;
    }

 protected:
    std::string _exchangeName;
    std::string _queueName;
    std::string _routingKey;
};

/********************************************************************************\
 * UnbindMessage
 ********************************************************************************/
class UnBindMessage : public RabbitMessageBase
{
    friend std::ostream& operator<< (std::ostream& ostream, const RabbitMessageBase& inst );
 public:

    UnBindMessage( const std::string & exchangeName, 
            const std::string & queueName,
            const std::string routingKey,
            AMQPConnection::OperationSucceededSetter operationSucceeded ) :
        RabbitMessageBase( operationSucceeded ),
        _exchangeName( exchangeName ),
        _queueName( queueName ),
        _routingKey( routingKey )
    { }

    MessageType messageType() const
    {
        return MessageType::UnBind;
    }

    const std::string & routingKey() const
    {
        return _routingKey;
    }

    const std::string & exchangeName() const
    {
        return _exchangeName;
    }

    const std::string & queueName()
    {
        return _queueName;
    }

 protected:
    std::string _exchangeName;
    std::string _queueName;
    std::string _routingKey;
};

} //namespace AMQP
#endif
