#ifndef CONNECTION_DETAILS_H
#define CONNECTION_DETAILS_H

#include <string>
#include <vector>

namespace AMQP {
struct RabbitConnectionDetails;
}

class ConnectionDetails
{
 public:
   ConnectionDetails(const char * userName,
       const char * password,
       const char * host,
       int port) :
     ConnectionDetails( std::string( userName), 
         std::string( password), 
         std::string( host ), 
         port )
  {}

   ConnectionDetails(const std::string& i_userName,
       const std::string& i_password,
       const std::string& i_host,
       int port) :
     _connectionData( i_userName,i_password,i_host,port )
  {
    _currentHost = _connectionData._hosts.end();
    _currentPort = _connectionData._ports.end();
  }

   ConnectionDetails( const ConnectionDetails & other ) :
     _connectionData( other._connectionData )
  {
    _currentHost = _connectionData._hosts.end();
    _currentPort = _connectionData._ports.end();
  }

   ConnectionDetails & operator = ( const ConnectionDetails & other )
   {
     _connectionData = other._connectionData;
     _currentHost = _connectionData._hosts.end();
     _currentPort = _connectionData._ports.end();
     return * this;
   }

   AMQP::RabbitConnectionDetails firstHost();
   AMQP::RabbitConnectionDetails nextHost();
   void reset();
   bool isLastHost () const;
   void addAlternateHost(const std::string& i_host);
   void addAlternatePort(int port);

 private:
   struct ConnectionDetailsData
   {
     ConnectionDetailsData( const std::string & userName,
         const std::string & password,
         const std::string & host,
         int port);

     std::string _userName;
     std::string _password;
     std::vector <  std::string > _hosts;
     std::vector <  int > _ports;
   };

   ConnectionDetailsData                  _connectionData;
   std::vector< std::string>::iterator    _currentHost;
   std::vector< int >::iterator           _currentPort;
};

#endif
