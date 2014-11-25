#include "connectionDetails.h"

#include <assert.h>

//TODO: add another ctor, that takes vecs.
connectionDetailsParam::connectionDetailsParam(
        const std::string& i_userName,
        const std::string& i_password,
        const std::string& i_host,
        int i_port) :
    m_userName(i_userName),
    m_password(i_password)
{
    m_hosts.push_back( i_host );
    m_ports.push_back( i_port );
}

connectionDetails::connectionDetails(const std::string& i_userName,
    const std::string& i_password,
    const std::string& i_host,
    int port) :
m_connectionParams(i_userName, i_password, i_host, port)
{
  m_currentHost = m_connectionParams.m_hosts.begin();
  m_currentPort = m_connectionParams.m_ports.begin();
}

const std::pair<std::string, int>  connectionDetails::getFirstHost()
{
    m_currentHost = m_connectionParams.m_hosts.begin();
    m_currentPort = m_connectionParams.m_ports.begin();
    return std::pair<std::string, int> (*m_currentHost, *m_currentPort);
}

const std::pair<std::string, int>  connectionDetails::getNextHost()
{
    ++m_currentHost;
        if (m_currentHost == m_connectionParams.m_hosts.end())
        {
            m_currentHost = m_connectionParams.m_hosts.begin();
            ++m_currentPort;
            if (m_currentPort == m_connectionParams.m_ports.end())
            {
                m_currentPort = m_connectionParams.m_ports.begin();
            }
        }
    return std::pair<std::string, int> (*m_currentHost, *m_currentPort);
}

bool connectionDetails::isLastHost () const
{
  assert (m_currentHost != m_connectionParams.m_hosts.end());
  assert (m_currentPort != m_connectionParams.m_ports.end());
  return next(m_currentHost) == m_connectionParams.m_hosts.end() &&
      next(m_currentPort) == m_connectionParams.m_ports.end();
}

std::string connectionDetails::createConnectionString( const connectionDetails& i_connectionDetails)
{
    std::stringstream ss;
    ss << i_connectionDetails.m_connectionParams.m_userName 
        << ":" 
        << i_connectionDetails.m_connectionParams.m_password 
        << "@" 
        << *(i_connectionDetails.m_currentHost)
        << ":" 
        << *(i_connectionDetails.m_currentPort);
    return ss.str();
}

