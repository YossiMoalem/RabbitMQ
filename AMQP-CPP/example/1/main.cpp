#include "myConnectionHandler.h"

#define EXC "exchange_name"
#define KEY1 "AdamKey"
#define QUEUE "AdamQueue"

void runConsumer()
{
    MyConnectionHandler connectionHandler( 'c' );
    connectionHandler.login();
    connectionHandler.declareExchange( EXC );
    connectionHandler.declareQueue( QUEUE );
    connectionHandler.bindQueueToExchange( KEY1 );
    while(1)
    {
        sleep( 1 );
        connectionHandler.receiveMessage();
    }
}

void runProducer()
{
    MyConnectionHandler connectionHandler( 'p' );
    connectionHandler.login();
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
