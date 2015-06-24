#ifndef CONNECTION_STATE_H
#define CONNECTION_STATE_H

#include "ResultCodes.h"

#include <assert.h>
#include <boost/noncopyable.hpp>

namespace AMQP{

class ConnectionState : boost::noncopyable
{
 public:
   ConnectionState ( std::function< void() >  onDisconnectCB ) :
       _currentConnectionState( CurrentConnectionState::Disconnected ),
       _onDisconnectCB( onDisconnectCB )
    {}

   bool disconnected()
   {
       if( _currentConnectionState == CurrentConnectionState::LoggingIn )
       {
           _loginResultSetter->set_value( false );
           _loginResultSetter.reset();
       }
       if( _currentConnectionState == CurrentConnectionState::Disconnecting &&
               _disconnectResultSetter != dummyResultSetter )
       {
           _disconnectResultSetter->set_value( true );
           _disconnectResultSetter.reset();
       }
       if( _currentConnectionState != CurrentConnectionState::Disconnected && 
               _currentConnectionState != CurrentConnectionState::SocketConnecting )
       {
           _onDisconnectCB();
       }
       _currentConnectionState = CurrentConnectionState::Disconnected;
       return true;
   }

   bool socketConnecting()
   {
       _currentConnectionState = CurrentConnectionState::SocketConnecting;
       return true;
   }

   bool socketConnected()
   {
       _currentConnectionState = CurrentConnectionState::SocketConnected;
       return true;
   }

   bool loggingIn( DeferedResultSetter loginResultSetter )
   {
       if( _currentConnectionState != CurrentConnectionState::LoggingIn )
       {
           _currentConnectionState = CurrentConnectionState::LoggingIn;
           _loginResultSetter = loginResultSetter;
           return true;
       } else {
           return false;
       }
   }

   bool loggedIn()
   {
       if( _currentConnectionState == CurrentConnectionState::LoggingIn )
       {
           _loginResultSetter->set_value( true );
           _loginResultSetter.reset();
       }
       _currentConnectionState = CurrentConnectionState::LoggedIn;
       return true;
   }

   bool disconnecting( DeferedResultSetter disconnectResultSetter )
   {
       if( _currentConnectionState == CurrentConnectionState::LoggingIn )
       {
           assert( disconnectResultSetter == dummyResultSetter);
           return true;
       }
       if( _currentConnectionState != CurrentConnectionState::Disconnecting )
       {
           _disconnectResultSetter = disconnectResultSetter;
       }
       _currentConnectionState = CurrentConnectionState::Disconnecting;
       return true;
   }

 private:
   enum class CurrentConnectionState
   {
       Disconnected         = 0,
       SocketConnecting     = 1,
       SocketConnected      = 2,
       LoggingIn            = 3,
       LoggedIn             = 4,
       Disconnecting        = 5,
   };

   CurrentConnectionState   _currentConnectionState;
   DeferedResultSetter      _loginResultSetter;
   DeferedResultSetter      _disconnectResultSetter;
   std::function< void() >  _onDisconnectCB;
};

} //namespace AMQP
#endif
