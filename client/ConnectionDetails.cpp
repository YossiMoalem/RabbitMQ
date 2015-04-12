#include "ConnectionDetails.h"

#include <assert.h>
#include <AmqpConnectionDetails.h>

ConnectionDetails::ConnectionDetailsData::ConnectionDetailsData( const std::string & userName,
            const std::string & password,
            const std::string & host,
            int port) :
  _userName( userName ),
  _password( password )
{ 
  _hosts.push_back( host );
  _ports.push_back( port );
}

void ConnectionDetails::addAlternateHost(const std::string& i_host)
{
    _connectionData._hosts.push_back(i_host);
}

void ConnectionDetails::addAlternatePort(int port)
{
    _connectionData._ports.push_back(port);
}

void ConnectionDetails::reset()
{
    _currentHost = _connectionData._hosts.end();
    _currentPort = _connectionData._ports.end();
}

AMQP::AmqpConnectionDetails ConnectionDetails::getFirstHost()
{
    _currentHost = _connectionData._hosts.begin();
    _currentPort = _connectionData._ports.begin();
    return AMQP::AmqpConnectionDetails( _connectionData._userName,
                                _connectionData._password,
                                *_currentHost,
                                *_currentPort );
}

AMQP::AmqpConnectionDetails ConnectionDetails::getNextHost()
{
  if (_currentHost == _connectionData._hosts.end() &&
      _currentPort == _connectionData._ports.end())
  {
    return getFirstHost();
  }

  ++_currentHost;
  if (_currentHost == _connectionData._hosts.end())
  {
    _currentHost = _connectionData._hosts.begin();
    ++_currentPort;
    if (_currentPort == _connectionData._ports.end())
    {
      _currentPort = _connectionData._ports.begin();
    }
  }
  return AMQP::AmqpConnectionDetails ( _connectionData._userName,
      _connectionData._password,
      *_currentHost,
      *_currentPort );
}

bool ConnectionDetails::isLastHost () const
{
  assert (_currentHost != _connectionData._hosts.end());
  assert (_currentPort != _connectionData._ports.end());
  return next(_currentHost) == _connectionData._hosts.end() &&
      next(_currentPort) == _connectionData._ports.end();
}
