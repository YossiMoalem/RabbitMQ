#include "ConnectionDetails.h"

#include <assert.h>

ConnectionDetails::ConnectionDetailsParam::ConnectionDetailsParam( const std::string & userName,
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
    _connectionParams._hosts.push_back(i_host);
}

void ConnectionDetails::addAlternatePort(int port)
{
    _connectionParams._ports.push_back(port);
}

ConnectionDetails::HostConnectionParams ConnectionDetails::getFirstHost()
{
    _currentHost = _connectionParams._hosts.begin();
    _currentPort = _connectionParams._ports.begin();
    return HostConnectionParams( _connectionParams._userName,
                                _connectionParams._password,
                                *_currentHost,
                                *_currentPort );
}

ConnectionDetails::HostConnectionParams ConnectionDetails::getNextHost()
{
  ++_currentHost;
  if (_currentHost == _connectionParams._hosts.end())
  {
    _currentHost = _connectionParams._hosts.begin();
    ++_currentPort;
    if (_currentPort == _connectionParams._ports.end())
    {
      _currentPort = _connectionParams._ports.begin();
    }
  }
  return HostConnectionParams( _connectionParams._userName,
      _connectionParams._password,
      *_currentHost,
      *_currentPort );
}

bool ConnectionDetails::isLastHost () const
{
  assert (_currentHost != _connectionParams._hosts.end());
  assert (_currentPort != _connectionParams._ports.end());
  return next(_currentHost) == _connectionParams._hosts.end() &&
      next(_currentPort) == _connectionParams._ports.end();
}
