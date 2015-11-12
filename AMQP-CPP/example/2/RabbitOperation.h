#ifndef RABBIT_MESSAGE_H
#define RABBIT_MESSAGE_H

#include "Types.h"
#include "amqpcpp.h"

namespace AMQP {
class RabbitJobHandler;

/********************************************************************************\
 * RabbitMessageBase
 ********************************************************************************/
class RabbitMessageBase
{
 public:

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

    void setHandler( RabbitJobHandler * handler )
    {
        _jobHandler = handler;
    }

    virtual void handle( ) = 0;

 protected:
    DeferedResultSetter     _returnValueSetter;
    RabbitJobHandler*      _jobHandler;
};

/********************************************************************************\
 * PostMessage
 ********************************************************************************/
class PostMessage : public RabbitMessageBase
{
 public:
    PostMessage( const std::string & exchangeName, 
            const std::string & routingKey, 
            const std::string & message ):
        _exchangeName( exchangeName ),
        _routingKey( routingKey ),
        _message( message )
    {}

    virtual void handle( ) override;

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
            const std::string routingKey ):
        _exchangeName( exchangeName ),
        _queueName( queueName ),
        _routingKey( routingKey )
    { }

    virtual void handle( ) override;

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

    virtual void handle( ) override;

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
    StopMessage( )
    { }

    virtual void handle( ) override;
};

/********************************************************************************\
 * LoginMessage
 ********************************************************************************/
class LoginMessage : public RabbitMessageBase
{
 public:
    LoginMessage( const std::string & userName, 
                const std::string & password ):
        _userName( userName ),
        _password( password )
    {}

    virtual void handle( ) override;

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
           bool durable ):
       _exchangeName( exchangeName ),
       _exchangeType( exchangetype ),
       _durable( durable )
    {}

    virtual void handle( ) override;

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
           bool autoDelete ):
       _queueName( queueName ),
       _durable( durable ),
       _exclusive( exclusive ),
       _autoDelete( autoDelete )
    {}

   virtual void handle( ) override;

 protected:
   std::string     _queueName;
   bool            _durable;
   bool            _exclusive;
   bool            _autoDelete;
};

/********************************************************************************\
 * RemoveQueue
 ********************************************************************************/
class RemoveQueueMessage : public RabbitMessageBase
{
 public:
   RemoveQueueMessage( const std::string & queueName ):
       _queueName( queueName )
    {}

   virtual void handle( ) override;

 protected:
   std::string     _queueName;
};

} //namespace AMQP
#endif
