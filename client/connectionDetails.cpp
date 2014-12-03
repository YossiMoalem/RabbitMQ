#include "connectionDetails.h"

#include <assert.h>

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

void connectionDetails::addHost(const std::string& i_host)
{
    m_connectionParams.m_hosts.push_back(i_host);
}

void connectionDetails::addPort(int port)
{
    m_connectionParams.m_ports.push_back(port);
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

std::string connectionDetails::createConnectionString() 
{
    std::stringstream ss;
    ss << m_connectionParams.m_userName 
        << ":" 
        << m_connectionParams.m_password 
        << "@" 
        << *(m_currentHost)
        << ":" 
        << *(m_currentPort);
    return ss.str();
}

