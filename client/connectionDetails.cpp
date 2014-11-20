#include "connectionDetails.h"

#include <assert.h>


connectionDetailsParam::connectionDetailsParam(
        const std::string& i_userName,
        const std::string& i_password,
        const std::string& i_host,
        int port) :
    m_userName(i_userName),
    m_password(i_password)
{
    std::pair<std::string, int> address (i_host, port);
    m_address.push_back(address);
}

connectionDetails::connectionDetails(const std::string& i_userName,
    const std::string& i_password,
    const std::string& i_host,
    int port) :
m_connectionParams(i_userName, i_password, i_host, port)
{
  m_currentAddress = m_connectionParams.m_address.begin();
}

const std::pair<std::string, int>&  connectionDetails::getFirstHost()
{
    m_currentAddress = m_connectionParams.m_address.begin();
    return *m_currentAddress;
}

const std::pair<std::string, int>&  connectionDetails::getNextHost()
{
  ++m_currentAddress;
  if (m_currentAddress == m_connectionParams.m_address.end())
  {
    m_currentAddress = m_connectionParams.m_address.begin();
  }
  return *m_currentAddress;
}

bool connectionDetails::isLastHost () const
{
  assert (m_currentAddress != m_connectionParams.m_address.end());
  return next(m_currentAddress) == m_connectionParams.m_address.end();
}

std::string connectionDetails::createConnectionString( const connectionDetails& i_connectionDetails)
{
    std::stringstream ss;
    ss << i_connectionDetails.m_connectionParams.m_userName 
        << ":" 
        << i_connectionDetails.m_connectionParams.m_password 
        << "@" 
        << i_connectionDetails.m_currentAddress->first
        << ":" 
        << i_connectionDetails.m_currentAddress->second;
    return ss.str();
}

