#include <amqpcpp.h>

#include <iostream> 
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include<thread>
#include <errno.h>

#define RABBIT_PORT 5672
#define RABBIT_IP1 "184.73.205.221"
#define RABBIT_IP2 "184.169.148.90"
#define KEY1 "key1"
#define KEY2 "key2"

#define QUEUE "YossiQueue"
#define EXC "exchange_name"
class MyConnectionHandler : public AMQP::ConnectionHandler
{

public:
    MyConnectionHandler( char type)
    {
        socketFd = socket( AF_INET, SOCK_STREAM, 0);
        if( socketFd < 0 )
        {
            std::cerr<<"Error creating Socket " <<std::endl;
        }


        sockaddr_in serv_addr = { 0 };
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons( RABBIT_PORT ); 
        if( inet_pton(AF_INET, ( type == 'c' ) ? RABBIT_IP1 : RABBIT_IP2 , &serv_addr.sin_addr) <= 0 )
        {
            std::cerr<<"inet_pton error occured" <<std::endl;
        }

        if( connect(socketFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            std::cerr <<"Error : Connect Failed "<< std::endl;
        } 

        onSocketConnected( type );
    }
    /**
     *  Method that is called by the AMQP library every time it has data
     *  available that should be sent to RabbitMQ.
     *  @param  connection  pointer to the main connection object
     *  @param  data        memory buffer with the data that should be sent to RabbitMQ
     *  @param  size        size of the buffer
     */
    virtual void onData(AMQP::Connection *connection, const char *data, size_t size)
    {
        std::cout <<"sending: ( " << size << " ):" << data <<std::endl;
        send( socketFd, data, size, 0);
    }

    /**
     *  Method that is called by the AMQP library when the login attempt
     *  succeeded. After this method has been called, the connection is ready
     *  to use.
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected( AMQP::Connection *connection )
    {
        std::cout << "AMQP login success" << std::endl;
         if (!_channel) _channel = new AMQP::Channel (_connection);
    }

    void handleResponse( )
    {
        const int buffsize = 1024;
        char buff[buffsize];
        bzero( buff, buffsize) ;
        int i = read( socketFd, buff, buffsize);
        std::cout <<"Got: " << i <<" Bytes" <<std::endl;
        _connection->parse( buff, i );
    }

    virtual void onSocketConnected(char type)
    {
        // report connection
        std::cout << "connected" << std::endl;
        // create amqp connection, and a new channel
        _connection = new AMQP::Connection(this, AMQP::Login("yossi", "yossipassword"), std::string( "/" ) );
        handleResponse( );
        handleResponse( );
        handleResponse( );
        handleResponse( );

        _channel = new AMQP::Channel(_connection);
        handleResponse( );

        // install a handler when channel is in error
        _channel->onError([](const char *message) {
                std::cout << "channel error " << message << std::endl;
                });

        // install a handler when channel is ready
        _channel->onReady([]() {
                std::cout << "channel ready" << std::endl;
                });

        // we declare a queue, an exchange and we publish a message
        _channel->declareQueue( QUEUE ).onSuccess([this]() { 
                std::cout << "queue declared" << std::endl; 
                // start consuming
                _channel->consume( QUEUE ).onReceived([ this ](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
                    std::cout << "received: " << message.message() << std::endl;
                    _channel->ack( deliveryTag );
                    });
                });
        handleResponse( );
        handleResponse( );

        // declare an exchange
        _channel->declareExchange( EXC, AMQP::topic).onSuccess([]() { 
                std::cout << "exchange declared" << std::endl; 
                });
        handleResponse();

        // bind queue and exchange
        _channel->bindQueue( EXC, QUEUE, (type == 'c' ? KEY1: KEY2) ).onSuccess([this, type]() {
                std::cout << "queue bound to exchange" << std::endl;
                });
        handleResponse( );

        while (1)
        {
            sleep(1);
            if (type == 'p')
            {
                std::cout <<"publishing tananainai" <<std::endl;
                _channel->publish(EXC, KEY1 , "Tananainai");
            } else {
            handleResponse( );
            }
        }
    }

    /**
     *  Method that is called by the AMQP library when a fatal error occurs
     *  on the connection, for example because data received from RabbitMQ
     *  could not be recognized.
     *  @param  connection      The connection on which the error occured
     *  @param  message         A human readable error message
     */
    virtual void onError(AMQP::Connection *connection, const char *message)
    {
        std::cout <<"Error: Error: "<< message <<std::endl;
    }

    /**
     *  Method that is called when the connection was closed. This is the
     *  counter part of a call to Connection::close() and it confirms that the
     *  connection was correctly closed.
     *
     *  @param  connection      The connection that was closed and that is now unusable
     */
    virtual void onClosed(AMQP::Connection *connection) 
    {
        std::cout <<"Info: Connection Closed"<< std::endl;
    }
 private:
    AMQP::Connection* _connection;
    AMQP::Channel * _channel;

    int socketFd;

};

void test(char type )
{
    MyConnectionHandler myConnectionHandler( type );
    sleep( 16 );
}
int main( int argc, char** argv )
{
    if( argc != 2 )
    {
        std::cerr <<"Usage: " << argv[0] <<" <Client ID (c/p) > " <<std::endl;
        exit(0);
    }
    char type = argv[1][0];
    std::thread t( test, type );
    t.join();
}
