#include "AMQPClient.h"
#include "AMQPConnectionDetails.h"
#include "Debug.h"

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

using namespace AMQP;

void runConsumer()
{
    AMQPConnectionDetails connectionDetails( USER, PASSWORD, RABBIT_IP2, RABBIT_PORT );
    AMQPClient amqpClient( [] ( const AMQP::Message & message ) {
            PRINT_DEBUG(DEBUG, "Consumer: Received: " << message.message() );
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
            PRINT_DEBUG(DEBUG, "Exchange Declared!");
        } else {
            PRINT_DEBUG(DEBUG, "Error declaring exchange");
            exit( 1 );
        }
        DeferedResult declareQueueResult = amqpClient.declareQueue( QUEUE );
        declareQueueResult.wait();
        if( declareQueueResult.get() )
        {
            PRINT_DEBUG(DEBUG, "Queue Declared!");
        } else {
            PRINT_DEBUG(DEBUG, "Error declaring queue");
            exit( 1 );
        }
        DeferedResult bindResult = amqpClient.bindQueue( EXC, QUEUE, KEY1 );
        bindResult.wait();

        if( bindResult.get() )
        {
            PRINT_DEBUG(DEBUG, "Queue Binded!");
        } else {
            PRINT_DEBUG(DEBUG, "Error binding queue");
            exit( 1 );
        }
        //        sleep( 5 );
        //amqpClient.stop( false );
    }
    amqpClient.waitForDisconnection();
}

void runProducer()
{
    AMQPConnectionDetails connectionDetails ( USER, PASSWORD, RABBIT_IP2, RABBIT_PORT );
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
            PRINT_DEBUG(DEBUG, "Exchange Declared!");
        } else {
            PRINT_DEBUG(DEBUG, "Error declaring exchange");
            exit( 1 );
        }
        for( int i = 0; i < 10; ++i)
        {
            std::string message = std::string( "tananainai" );
            message += ( std::to_string( i ) );
            amqpClient.publish(exchangeName, KEY1, message );
        }
        sleep(5);
        PRINT_DEBUG(DEBUG, "Calling stop");
        amqpClient.stop(false);
        PRINT_DEBUG(DEBUG, "stoped");
    }
    amqpClient.waitForDisconnection();
}

#define showUsage \
do { \
    PRINT_DEBUG(DEBUG, "Usage: " << argv[0] <<" <Client type (c/p) > "); \
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
