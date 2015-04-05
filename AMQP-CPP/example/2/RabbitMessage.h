#ifndef RABBIT_MESSAGE_H
#define RABBIT_MESSAGE_H

#include <iostream>
#include <sstream>
#include <memory>
#include <future>

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
   typedef std::shared_ptr< std::promise< bool > > OperationSucceededSetter;

    RabbitMessageBase( ):
        _returnValueSetter( new std::promise< bool > )
    {}
    
    virtual ~RabbitMessageBase () {}

    RabbitMessageBase( const RabbitMessageBase& ) = delete;
    RabbitMessageBase& operator= (const RabbitMessageBase& ) = delete;

    OperationSucceededSetter resultSetter()
    {
        return _returnValueSetter;
    }

    std::future< bool > deferedResult()
    {
        return _returnValueSetter->get_future();
    }

    virtual MessageType messageType() const = 0;

 protected:
    OperationSucceededSetter _returnValueSetter;

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
            const std::string & message) :
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
            const std::string routingKey ) :
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
            const std::string routingKey ) :
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
