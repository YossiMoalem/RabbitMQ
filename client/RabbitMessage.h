#ifndef RABBIT_MESSAGE_H
#define RABBIT_MESSAGE_H

#include <iostream>
#include <sstream>

#include "Types.h"

#include "internalTypes.h"

/********************************************************************************\
 * RabbitMessageBase
 ********************************************************************************/
class RabbitMessageBase
{
    friend std::ostream& operator<< (std::ostream& ostream, const RabbitMessageBase& inst );
 public:
    RabbitMessageBase (
            const std::string& i_destination,
            DeliveryType i_deliveryType) :
        m_destination(i_destination),
        m_deliveryType(i_deliveryType)
    { };
    virtual ~RabbitMessageBase() {}

    RabbitMessageBase( const RabbitMessageBase& ) = delete;
    RabbitMessageBase( const RabbitMessageBase&& ) = delete;
    RabbitMessageBase& operator= (const RabbitMessageBase& ) = delete;
    RabbitMessageBase& operator= (const RabbitMessageBase&& ) = delete;

    static RabbitMessageBase* deserialize (const std::string& i_serializes);
    std::string serialize () const ;

    virtual std::string getRoutingKey() const = 0;
    virtual MessageType messageType() const = 0;
    std::string m_destination;

 protected:
    virtual void doSerialize (std::stringstream& o_serializer) const = 0;
    virtual std::string toString () const = 0;

 protected:
    DeliveryType m_deliveryType;
};

/********************************************************************************\
 * PostMessage
 ********************************************************************************/
class PostMessage : public RabbitMessageBase
{
    friend std::ostream& operator<< (std::ostream& ostream, const RabbitMessageBase& inst );
 public:
    PostMessage( const std::string& i_text,
            const std::string& i_destination,
            const std::string& i_senderID,
            DeliveryType i_deliveryType) :
        RabbitMessageBase ( i_destination, i_deliveryType),
        m_sender(i_senderID),
        m_text(i_text)
    { };

    void doSerialize (std::stringstream& o_serializer) const;
    static PostMessage* doDeserialize (const std::string& i_serialized);
    virtual std::string toString () const;

    const std::string getText () const
    {
        return m_text;
    }
    MessageType messageType() const
    {
        return MessageType::Post;
    }

    DeliveryType deliveryType() const
    {
        return m_deliveryType;
    }

    std::string getRoutingKey() const;

 public:
    std::string m_sender;

 protected:
    std::string m_text;
};

/********************************************************************************\
 * UnbindMessage
 ********************************************************************************/
class UnbindMessage : public RabbitMessageBase
{
    friend std::ostream& operator<< (std::ostream& ostream, const RabbitMessageBase& inst );
 public:

    UnbindMessage( const std::string& i_key,
            const std::string& i_destination,
            DeliveryType i_deliveryType) :
        RabbitMessageBase (i_destination, i_deliveryType),
        m_key(i_key)
    { };

    static UnbindMessage* doDeserialize (const std::string& i_serialized);
    MessageType messageType() const;
    void doSerialize (std::stringstream& o_serializer) const;
    virtual std::string toString () const;
    std::string getRoutingKey() const;
    std::string unbindKey() const;

 protected:
    std::string m_key;
};

/********************************************************************************\
 * BindMessage
 ********************************************************************************/
class BindMessage : public RabbitMessageBase
{
    friend std::ostream& operator<< (std::ostream& ostream, const RabbitMessageBase& inst );
 public:
    BindMessage( const std::string& i_key,
            const std::string& i_destination,
            DeliveryType i_deliveryType) :
        RabbitMessageBase (i_destination, i_deliveryType),
        m_key(i_key)
    { };

    static BindMessage* doDeserialize (const std::string& i_serialized);
    MessageType messageType() const
    {
        return MessageType::Bind;
    }

    std::string bindKey() const;
    void doSerialize (std::stringstream& o_serializer) const;
    virtual std::string toString () const;
    std::string getRoutingKey() const;

 protected:
    std::string m_key;
};

#endif
