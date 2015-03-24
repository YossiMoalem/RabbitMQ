#include "myConnectionHandler.h"
#include "AmqpConnectionDetails.h"

#define EXC "exchange_name"
#define KEY1 "YossiKey"
#define QUEUE "YossiQueue"

#define RABBIT_PORT 5672
#define RABBIT_IP1 "184.73.205.221"
#define RABBIT_IP2 "184.169.148.90"
#define USER "yossi"
#define PASSWORD "yossipassword"


void runConsumer()
{
    AmqpConnectionDetails connectionDetails( USER, PASSWORD, RABBIT_IP1, RABBIT_PORT );
    MyConnectionHandler connectionHandler( [] ( const AMQP::Message & message ) {
            std::cout <<"Received: " << message.message() << std::endl ;
            return 0; } );
    if( connectionHandler.login( connectionDetails ) )
    {
        connectionHandler.declareExchange( EXC );
        connectionHandler.declareQueue( QUEUE );
        connectionHandler.bindQueueToExchange( KEY1 );
        while(1)
        {
            connectionHandler.receiveMessage();
        }
    }
}

void runProducer()
{
    AmqpConnectionDetails connectionDetails ( USER, PASSWORD, RABBIT_IP2, RABBIT_PORT );
    MyConnectionHandler connectionHandler( nullptr );
    connectionHandler.login( connectionDetails );
    connectionHandler.declareExchange( EXC );
    while(1)
    {
        sleep( 1 );
        connectionHandler.publish(KEY1, "tananainai" );
    }
}

#define showUsage \
do { \
    std::cerr <<"Usage: " << argv[0] <<" <Client type (c/p) > " <<std::endl; \
    exit(0); \
} while( 0 );

int main( int argc, char** argv )
{
    if( argc != 2 )
    {
        showUsage;
    }

    char type = argv[1][0];
    if( type == 'c' )
        runConsumer();
    else if( type == 'p' )
        runProducer();
    else
        showUsage;
}
