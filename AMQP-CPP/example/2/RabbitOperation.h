#ifndef RABBIT_MESSAGE_H
#define RABBIT_MESSAGE_H

#include <memory>
#include <future>
#include <amqpcpp.h>

namespace AMQP {

class AMQPEventLoop;
typedef std::future< bool > DeferedResult;

/********************************************************************************\
 * RabbitMessageBase
 ********************************************************************************/
class RabbitMessageBase
{
 public:
   typedef std::shared_ptr< std::promise< bool > > DeferedResultSetter;

    RabbitMessageBase( ):
        _returnValueSetter( new std::promise< bool > )
    { }
    
    virtual ~RabbitMessageBase () {}

    RabbitMessageBase( const RabbitMessageBase& ) = delete;
    RabbitMessageBase& operator= (const RabbitMessageBase& ) = delete;

    DeferedResultSetter resultSetter()
    {
        return _returnValueSetter;
    }

    DeferedResult deferedResult()
    {
        return _returnValueSetter->get_future();
    }

    virtual void handle( AMQPEventLoop * eventLoop ) = 0;

 protected:
    DeferedResultSetter    _returnValueSetter;
};

/********************************************************************************\
 * PostMessage
 ********************************************************************************/
class PostMessage : public RabbitMessageBase
{
 public:
    PostMessage( const std::string & exchangeName, 
            const std::string & routingKey, 
            const std::string & message) :
        _exchangeName( exchangeName ),
        _routingKey( routingKey ),
        _message( message )
    {}

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

    virtual void handle( AMQPEventLoop * eventLoop ) override;

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
 public:
    BindMessage( const std::string & exchangeName, 
            const std::string & queueName,
            const std::string routingKey ) :
        _exchangeName( exchangeName ),
        _queueName( queueName ),
        _routingKey( routingKey )
    { }

    const std::string & routingKey() const
    {
        return _routingKey;
    }

    const std::string & exchangeName() const
    {
        return _exchangeName;
    }

    const std::string & queueName() const
    {
        return _queueName;
    }

    virtual void handle( AMQPEventLoop * eventLoop ) override;

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
 public:
    UnBindMessage( const std::string & exchangeName, 
            const std::string & queueName,
            const std::string routingKey ) :
        _exchangeName( exchangeName ),
        _queueName( queueName ),
        _routingKey( routingKey )
    { }

    const std::string & routingKey() const
    {
        return _routingKey;
    }

    const std::string & exchangeName() const
    {
        return _exchangeName;
    }

    const std::string & queueName() const
    {
        return _queueName;
    }

    virtual void handle( AMQPEventLoop * eventLoop ) override;

 protected:
    std::string _exchangeName;
    std::string _queueName;
    std::string _routingKey;
};

/********************************************************************************\
 * StopMessage
 ********************************************************************************/
class StopMessage : public RabbitMessageBase
{
 public:
    StopMessage( bool immediate ) :
        _immediate( immediate )
    { }

    bool terminateNow()
    {
        return _immediate;
    }

    void setTerminateNow()
    {
        _immediate = true;
    }

    virtual void handle( AMQPEventLoop * eventLoop ) override;

 protected:
    bool _immediate;
};

/********************************************************************************\
 * LoginMessage
 ********************************************************************************/
class LoginMessage : public RabbitMessageBase
{
 public:
    LoginMessage( const std::string & userName, 
                const std::string & password ) :
        _userName( userName ),
        _password( password )
    {}

    virtual void handle( AMQPEventLoop * eventLoop ) override;

 protected:
    std::string _userName;
    std::string _password;
};

/********************************************************************************\
 * DeclareExchange
 ********************************************************************************/
class DeclareExchangeMessage : public RabbitMessageBase
{
 public:
   DeclareExchangeMessage( const std::string & exchangeName, 
           ExchangeType exchangetype, 
           bool durable ) :
       _exchangeName( exchangeName ),
       _exchangeType( exchangetype ),
       _durable( durable )
    {}

    virtual void handle( AMQPEventLoop * eventLoop ) override;

 protected:
   std::string      _exchangeName;
   ExchangeType     _exchangeType;
   bool             _durable;
};

/********************************************************************************\
 * DeclareQueue
 ********************************************************************************/
class DeclareQueueMessage : public RabbitMessageBase
{
 public:
   DeclareQueueMessage( const std::string & queueName, 
           bool durable,
           bool exclusive, 
           bool autoDelete) :
       _queueName( queueName ),
       _durable( durable ),
       _exclusive( exclusive ),
       _autoDelete( autoDelete )
    {}

   virtual void handle( AMQPEventLoop * eventLoop ) override;

 protected:
   std::string     _queueName;
   bool            _durable;
   bool            _exclusive;
   bool            _autoDelete;
};

} //namespace AMQP
#endif
