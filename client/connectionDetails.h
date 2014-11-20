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
   std::vector < std::pair< std::string, int> >m_address;
};

class connectionDetails
{
 public:
   connectionDetails(const std::string& i_userName,
                    const std::string& i_password,
                    const std::string& i_host,
                    int port) ;
   const std::pair<std::string, int>&  getFirstHost();
   const std::pair<std::string, int>&  getNextHost();
   bool isLastHost () const;

   static std::string createConnectionString( const connectionDetails& i_connectionDetails);

private:
   connectionDetailsParam m_connectionParams;
   std::vector< std::pair< std::string, int> >::iterator m_currentAddress;

};

#endif
