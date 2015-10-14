#ifndef CONNECTION_DETAILS_H
#define CONNECTION_DETAILS_H

#include <string>
#include <vector>

namespace AMQP {
class RabbitConnectionDetails;
}

class ConnectionDetails
{
 public:
    ConnectionDetails( const ConnectionDetails & other ) :
        _connectionData( other._connectionData )
    {
        _currentHost = _connectionData._hosts.end();
        _currentPort = _connectionData._ports.end();
    }

    ConnectionDetails(const char * i_userName,
            const char * i_password,
            const char * i_host,
            int port) :
        _connectionData( std::string( i_userName), 
                std::string( i_password), 
                std::string( i_host ), 
                port )
    {
        _currentHost = _connectionData._hosts.end();
        _currentPort = _connectionData._ports.end();
    }

    ConnectionDetails(const std::string& i_userName,
            const std::string& i_password,
            const std::string& i_host,
            int port) :
        _connectionData( i_userName,i_password,i_host,port )
    {}

    AMQP::RabbitConnectionDetails getFirstHost();
    AMQP::RabbitConnectionDetails getNextHost();
    void reset();
    bool isLastHost () const;
    void addAlternateHost(const std::string& i_host);
    void addAlternatePort(int port);

 private:
    struct ConnectionDetailsData
    {
        friend class ConnectionDetails;
     private:
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
