#ifndef AMQP_HEARTBEAT_H
#define AMQP_HEARTBEAT_H

#include <boost/noncopyable.hpp>

namespace AMQP{

class AMQPConnectionHandler;

class Heartbeat : boost::noncopyable
{
 public:
   Heartbeat( AMQPConnectionHandler * connectionHandler );
   void invalidate();
   bool send();
   void initialize();
   void reset ();

 private:
   AMQPConnectionHandler *  _connectionHandler;
   bool                     _initialized = false;
   bool                     _heartbeatSent = false;

};

}//namespace AMQP
#endif
