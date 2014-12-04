#include "RabbitMessage.h"
#include "internalTypes.h"
#include <cstdlib>

#define UNICAST_PREFIX "ALL:"
#define MULTICAST_SUFFIX ":ALL"

/********************************************************************************\
 * RabbitMessageBase
 ********************************************************************************/
RabbitMessageBase* RabbitMessageBase::deserialize (const std::string& i_serialized)
{
    std::istringstream istream(i_serialized);
    int messageType;
    istream>> messageType;
    std::size_t messageTypeEndOps = i_serialized.find (',') + 1;
    switch (messageType)
    {
        case (int)MessageType::Post:
            return PostMessage::doDeserialize( i_serialized.substr(messageTypeEndOps) );
        case (int)MessageType::Bind:
            return BindMessage::doDeserialize( i_serialized.substr(messageTypeEndOps) );
        case (int)MessageType::Unbind:
            return UnbindMessage::doDeserialize( i_serialized.substr(messageTypeEndOps) );
        default:
            return nullptr;
    }
}

std::string RabbitMessageBase::serialize () const
{
    std::stringstream res;
    res << (int) messageType() << ",";
    doSerialize(res);
    return res.str();
}

std::ostream& operator<< ( std::ostream& ostream, const RabbitMessageBase& inst )
{
    return ostream <<inst.toString();
}

/********************************************************************************\
 * BindMessage
 ********************************************************************************/
BindMessage* BindMessage::doDeserialize (const std::string& i_serialized)
{
    std::istringstream i_deserializer(i_serialized);
    int deliveryType;
    std::string destination;
    std::string key;
    i_deserializer >>deliveryType;
    getline(i_deserializer, destination);
    getline(i_deserializer, key);

    return new BindMessage( key, destination, (DeliveryType)deliveryType);
}

void BindMessage::doSerialize (std::stringstream& o_serializer) const
{
    o_serializer << (int)m_deliveryType
        << m_destination << std::endl
        <<m_key<<std::endl;
}
std::string BindMessage::getRoutingKey() const
{
    return UNICAST_PREFIX + m_destination;
}

std::string BindMessage::bindKey() const
{
    if (m_deliveryType == DeliveryType::Unicast)
        return UNICAST_PREFIX + m_key;
    return m_key+ MULTICAST_SUFFIX;
}

std::string BindMessage::toString() const
{
    return std::string ("Bind " + getRoutingKey() + " To " + bindKey() );
}

/********************************************************************************\
 * UnbindMessage
 ********************************************************************************/
UnbindMessage* UnbindMessage::doDeserialize (const std::string& i_serialized)
{
    std::istringstream i_deserializer(i_serialized);
    int deliveryType;
    std::string destination;
    std::string key;

    i_deserializer >>deliveryType;
    getline(i_deserializer, destination);
    getline(i_deserializer, key);
    return new UnbindMessage( key, destination, (DeliveryType)deliveryType);
}

MessageType UnbindMessage::messageType() const
{
    return MessageType::Unbind;
}

void UnbindMessage::doSerialize (std::stringstream& o_serializer) const
{
    o_serializer<< (int)m_deliveryType
        << m_destination << std::endl
        <<m_key;
}

std::string UnbindMessage::unbindKey() const
{
    if (m_deliveryType == DeliveryType::Unicast)
        return UNICAST_PREFIX + m_key;
    return m_key + MULTICAST_SUFFIX;
}

std::string UnbindMessage::getRoutingKey() const
{
    return UNICAST_PREFIX + m_destination;
}

std::string UnbindMessage::toString() const
{
    return std::string ("Unbind " + getRoutingKey() + " From " + unbindKey() );
}

/********************************************************************************\
 * PostMessage
 ********************************************************************************/
PostMessage* PostMessage::doDeserialize (const std::string& i_serialized)
{
    std::istringstream i_deserializer(i_serialized);
    int deliveryType;
    std::string sender;
    std::string destination;
    std::string text;
    i_deserializer >>deliveryType;
    getline(i_deserializer, sender);
    getline(i_deserializer, destination);
    int lastTokenPos = i_deserializer.tellg();
    text = (i_deserializer.str()).substr(lastTokenPos);
    return new PostMessage( text, destination, sender, (DeliveryType)deliveryType);
}

void PostMessage::doSerialize (std::stringstream& o_serializer) const
{
    o_serializer << (int)m_deliveryType
        << m_sender << std::endl
        << m_destination << std::endl
        << m_text;
}

std::string PostMessage::getRoutingKey() const
{
    if (m_deliveryType == DeliveryType::Unicast)
        return UNICAST_PREFIX + m_destination;
    return m_sender + MULTICAST_SUFFIX;
}

std::string PostMessage::toString() const
{
    return std::string("Send :" + getText() + " From " + m_sender + " To " + getRoutingKey() );
}

#undef UNICAST_PREFIX
#undef MULTICAST_SUFFIX
