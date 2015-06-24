#include "AMQPClient.h"
#include "AMQPConnectionDetails.h"

#include <thread>
#include <unistd.h>
#include <string>

#define EXC "yossiExchange"
#define KEY1 "YossiKey"
#define QUEUE "YossiQueue"

#define RABBIT_PORT 5672
#define RABBIT_IP2 "184.73.205.221"
#define RABBIT_IP1 "184.169.148.90"
#define USER "yossi"
#define PASSWORD "yossipassword"

//TODO: remove
#include "RabbitOperation.h"

using namespace AMQP;

void runConsumer()
{
    AMQPConnectionDetails connectionDetails( USER, PASSWORD, RABBIT_IP1, RABBIT_PORT );
    AMQPClient amqpClient( [] ( const AMQP::Message & message ) {
            std::cout <<"Consumer: Received: " << message.message() << std::endl ;
            return 0; } );
    amqpClient.init( connectionDetails );
    DeferedResult loginStatus = amqpClient.login();
    loginStatus.wait();
    if ( loginStatus.get() )
    {
        std::string exchangeName( EXC );
        DeferedResult declareExchangeResult = amqpClient.declareExchange( exchangeName, 
                AMQP::topic );
        declareExchangeResult.wait();
        if( declareExchangeResult.get() )
        {
            std::cout <<"Exchange Declared!" <<std::endl;
        } else {
            std::cout <<"Error declaring exchange" <<std::endl;
            exit( 1 );
        }
        DeferedResult declareQueueResult = amqpClient.declareQueue( QUEUE );
        declareQueueResult.wait();
        if( declareQueueResult.get() )
        {
            std::cout <<"Queue Declared!" <<std::endl;
        } else {
            std::cout <<"Error declaring queue" <<std::endl;
            exit( 1 );
        }
        DeferedResult bindResult = amqpClient.bindQueue( EXC, QUEUE, KEY1 );
        bindResult.wait();

        if( bindResult.get() )
        {
            std::cout <<"Queue Binded!" <<std::endl;
        } else {
            std::cout <<"Error binding queue" <<std::endl;
            exit( 1 );
        }
        //        sleep( 5 );
        //amqpClient.stop( false );
    }
    amqpClient.waitForDisconnection();
}

void runProducer()
{
    AMQPConnectionDetails connectionDetails ( USER, PASSWORD, RABBIT_IP1, RABBIT_PORT );
    AMQPClient amqpClient( nullptr );
    amqpClient.init( connectionDetails );
    DeferedResult loginStatus = amqpClient.login();
    loginStatus.wait();
    if ( loginStatus.get() )
    {

        std::string exchangeName( EXC );
        DeferedResult declareExchangeResult = amqpClient.declareExchange( exchangeName, 
                AMQP::topic );
        declareExchangeResult.wait();
        if( declareExchangeResult.get() )
        {
            std::cout <<"Exchange Declared!" <<std::endl;
        } else {
            std::cout <<"Error declaring exchange" <<std::endl;
            exit( 1 );
        }
        for( int i = 0; i < 10; ++i)
        {
            std::string message = std::string( "tananainai" );
            message += ( std::to_string( i ) );
            amqpClient.publish(exchangeName, KEY1, message );
        }
        sleep(5);
        std::cout <<"Calling stop" <<std::endl;
        amqpClient.stop(false);
        std::cout <<"stoped" <<std::endl;
    }
    amqpClient.waitForDisconnection();
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
