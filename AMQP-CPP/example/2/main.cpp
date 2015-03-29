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

using namespace AMQP;

void runConsumer()
{
    AmqpConnectionDetails connectionDetails( USER, PASSWORD, RABBIT_IP1, RABBIT_PORT );
    MyConnectionHandler connectionHandler( [] ( const AMQP::Message & message ) {
            std::cout <<"Received: " << message.message() << std::endl ;
            return 0; } );
    if( connectionHandler.login( connectionDetails ) )
    {
        std::string exchangeName( EXC );
        MyConnectionHandler::OperationSucceeded declareExchangeResult = connectionHandler.declareExchange( exchangeName, AMQP::topic );
        declareExchangeResult.wait();
        if( declareExchangeResult.get() )
        {
            std::cout <<"Exchange Declared!" <<std::endl;
        } else {
            std::cout <<"Error declaring exchange" <<std::endl;
            exit( 1 );
        }
        MyConnectionHandler::OperationSucceeded declareQueueResult = connectionHandler.declareQueue( QUEUE );
        declareQueueResult.wait();
        if( declareQueueResult.get() )
        {
            std::cout <<"Queue Declared!" <<std::endl;
        } else {
            std::cout <<"Error declaring queue" <<std::endl;
            exit( 1 );
        }
        MyConnectionHandler::OperationSucceeded bindResult = connectionHandler.bindQueue( EXC, QUEUE, KEY1 );
        bindResult.wait();
        if( bindResult.get() )
        {
            std::cout <<"Queue Binded!" <<std::endl;
        } else {
            std::cout <<"Error binding queue" <<std::endl;
            exit( 1 );
        }
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
    std::string exchangeName( EXC );
    connectionHandler.declareExchange( exchangeName, AMQP::topic );
    while(1)
    {
        sleep( 1 );
        connectionHandler.publish(exchangeName, KEY1, "tananainai" );
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
