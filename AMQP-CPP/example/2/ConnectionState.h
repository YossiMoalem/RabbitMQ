#ifndef CONNECTION_STATE_H
#define CONNECTION_STATE_H

#include "RabbitOperation.h"

#include <boost/noncopyable.hpp>

namespace AMQP{

class ConnectionState : boost::noncopyable
{
 public:
   ConnectionState ( std::function< void() >  onDisconnectCB ) :
       _currentConnectionState( CurrentConnectionState::Disconnected ),
       _onDisconnectCB( onDisconnectCB )
    {}

   void disconnected()
   {
       if( _currentConnectionState == CurrentConnectionState::LoggingIn )
       {
           _loginResultSetter->set_value( false );
           _loginResultSetter.reset();
       }
       _onDisconnectCB();
       _currentConnectionState = CurrentConnectionState::SocketConnecting;
   }

   void socketConnecting()
   {
       _currentConnectionState = CurrentConnectionState::SocketConnecting;
   }

   void socketConnected()
   {
       _currentConnectionState = CurrentConnectionState::SocketConnected;
   }

   void loggingIn( DeferedResultSetter loginResultSetter )
   {
       _currentConnectionState = CurrentConnectionState::LoggingIn;
       _loginResultSetter = loginResultSetter;
   }

   void loggedIn()
   {
       if( _currentConnectionState == CurrentConnectionState::LoggingIn )
       {
           _loginResultSetter->set_value( true );
           _loginResultSetter.reset();
       }
       _currentConnectionState = CurrentConnectionState::LoggedIn;
   }

 private:
   enum class CurrentConnectionState
   {
       Disconnected         = 0,
       SocketConnecting     = 1,
       SocketConnected      = 2,
       LoggingIn            = 3,
       LoggedIn             = 4
   };

   CurrentConnectionState   _currentConnectionState;
   DeferedResultSetter      _loginResultSetter;
   std::function< void() >  _onDisconnectCB;
};

} //namespace AMQP
#endif
