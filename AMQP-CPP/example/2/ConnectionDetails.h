#ifndef CONNECTION_DETAILS_H
#define CONNECTION_DETAILS_H

#include <utility> //for pair
#include <string>
#include <vector>


class ConnectionDetails
{
 public:
    struct HostConnectionParams
    {
     public:
       HostConnectionParams(const std::string& i_userName,
               const std::string& i_password,
               const std::string& i_host,
               int port) :
           _userName( i_userName), 
           _password( i_password), 
           _host( i_host),
           _port( port )
        {}

       std::string  _userName;
       std::string  _password;
       std::string  _host;
       int          _port;
    };

    ConnectionDetails(const char * i_userName,
            const char * i_password,
            const char * i_host,
            int port) :
        _connectionParams( std::string( i_userName), 
                std::string( i_password), 
                std::string( i_host ), 
                port )
    {}

    ConnectionDetails(const std::string& i_userName,
            const std::string& i_password,
            const std::string& i_host,
            int port) :
        _connectionParams ( i_userName,i_password,i_host,port )
    {}

    HostConnectionParams getFirstHost();
    HostConnectionParams getNextHost();
    bool isLastHost () const;
    void addAlternateHost(const std::string& i_host);
    void addAlternatePort(int port);

 private:
    struct ConnectionDetailsParam
    {
        friend class ConnectionDetails;
     private:
        ConnectionDetailsParam( const std::string & userName,
            const std::string & password,
            const std::string & host,
            int port);

        std::string _userName;
        std::string _password;
        std::vector <  std::string > _hosts;
        std::vector <  int > _ports;
    };

    ConnectionDetailsParam                  _connectionParams;
    std::vector< std::string>::iterator     _currentHost;
    std::vector< int >::iterator            _currentPort;
};

#endif
