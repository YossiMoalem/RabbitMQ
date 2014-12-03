#ifndef CONNECTION_DETAILS_H
#define CONNECTION_DETAILS_H

#include <utility> //for pair
#include <string>
#include <vector>
#include <sstream>

struct connectionDetailsParam
{
   public:
   connectionDetailsParam(const std::string& i_userName,
                        const std::string& i_password,
                        const std::string& i_host,
                        int port) ;

   std::string m_userName;
   std::string m_password;
   std::vector <  std::string > m_hosts;
   std::vector <  int > m_ports;
};

class connectionDetails
{
 public:
   connectionDetails(const std::string& i_userName,
                    const std::string& i_password,
                    const std::string& i_host,
                    int port) ;

   const std::pair<std::string, int>  getFirstHost();
   const std::pair<std::string, int>  getNextHost();
   bool isLastHost () const;
   void addHost(const std::string& i_host);
   void addPort(int port);

   std::string createConnectionString( );

private:
   connectionDetailsParam m_connectionParams;
   std::vector< std::string>::iterator m_currentHost;
   std::vector< int >::iterator  m_currentPort;

};

#endif
