#include <iostream>
#include <sstream>

#include "common.h"

/********************************************************************************\
 * RabbitMessageBase
 ********************************************************************************/
class RabbitMessageBase
{
 public:
   RabbitMessageBase (
           const std::string i_destination,
           DeliveryType i_deliveryType) :
       m_deliveryType(i_deliveryType),
       m_destination(i_destination)
    { };
   virtual ~RabbitMessageBase() {}


   static RabbitMessageBase* desirialize (const std::string& i_serializes);
   std::string serialize () const ;

   virtual std::string getDestination() const = 0;
   virtual MessageType messageType() const = 0;
   virtual std::string toString () const = 0;

 protected:
   virtual void doSerialize (std::stringstream& o_serializer) const = 0;

 protected:
   DeliveryType m_deliveryType;
   std::string m_destination;
};

/********************************************************************************\
 * PostMessage
 ********************************************************************************/
class PostMessage : public RabbitMessageBase
{
 public:
   PostMessage( const std::string i_text, 
           const std::string i_destination,
           const std::string i_senderID,
           DeliveryType i_deliveryType) :
       RabbitMessageBase ( i_destination, i_deliveryType),
       m_sender(i_senderID),
       m_text(i_text)
    { };

   void doSerialize (std::stringstream& o_serializer) const;
   static PostMessage* doDeserialize (const std::string& i_serialized);
   std::string getDestination() const;
   virtual std::string toString () const;

   const std::string getText () const
   {
       return m_text;
   }
   MessageType messageType() const
   {
     return MessageType::Post;
   }

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
 public:

   UnbindMessage( const std::string i_key, 
           const std::string i_destination,
           DeliveryType i_deliveryType) :
       RabbitMessageBase (i_destination, i_deliveryType),
       m_key(i_key)
    { };

   static UnbindMessage* doDeserialize (const std::string& i_serialized);
   MessageType messageType() const;
   void doSerialize (std::stringstream& o_serializer) const;
   virtual std::string toString () const;

   std::string getDestination() const
   {
       return "ALL_" + m_destination;
   }

   std::string unbindKey() const;

 protected:
   std::string m_key;
};

/********************************************************************************\
 * BindMessage
 ********************************************************************************/
class BindMessage : public RabbitMessageBase
{
 public:
   BindMessage( const std::string i_key, 
           const std::string i_destination,
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

   std::string getDestination() const
   {
       return "ALL_" + m_destination;
   }
 protected:
   std::string m_key;
};
