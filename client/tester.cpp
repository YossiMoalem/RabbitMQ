#include "RabbitClient.h"

#include <sys/time.h>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <boost/lexical_cast.hpp>

#define MY_NAME "USR1"
#define OTHER_NAME "Kuku"
//#define RABBIT_IP1 "test.mb.hubs.liveu.tv"
#define RABBIT_IP1 "75.101.155.196"
#define RABBIT_PORT 5672
#define USER "hub1"
#define PASSWORD "hub1password"
#define EXCHANGE_NAME "EXC1"

static const unsigned int timeToConnect = 2;
static const unsigned int timeToBindUnbind = 3;
static const unsigned int timeToFlushAllMessages = 4;

/*****************************************************************************\
 *  Bind Tester 
\*****************************************************************************/
class BindTester
{
 public:

   int operator ()()
   {
     ConnectionDetails cnd( USER, PASSWORD, RABBIT_IP1, RABBIT_PORT );
       RabbitClient client (cnd, EXCHANGE_NAME, MY_NAME, [] ( std::string o_sender, std::string destination, DeliveryType, std::string message )->int {
               std::cout <<"Free CB:: Received: "<<message 
                        <<" From : "<< o_sender << std::endl;
               return 0;
               } );
       client.start();
       sleep (timeToConnect);
       test(client);
       client.stop(false);
       return 0;
   }

   static int test (RabbitClient& client)
   {
       RABBIT_DEBUG ( "Tester:: Tester started ");
       RABBIT_DEBUG ("-------------------------------------------------------");
       RABBIT_DEBUG ("Tester::  + Send message to relevant/nonrelevane queue:");
       RABBIT_DEBUG ("Tester:: Going to send 6 lalalila messages to the relevant queue. Should be recieved");
       client.bindToSelf (MY_NAME);
       while ( client.sendUnicast(std::string("lalalila1"), MY_NAME, MY_NAME, EXCHANGE_NAME ) != ReturnStatus::Ok ) sleep(1);
       client.sendUnicast(std::string("lalalila2"), MY_NAME, MY_NAME, EXCHANGE_NAME );
       client.sendUnicast(std::string("lalalila3"), MY_NAME, MY_NAME, EXCHANGE_NAME );
       client.sendUnicast(std::string("lalalila4"), MY_NAME, MY_NAME, EXCHANGE_NAME );
       client.sendUnicast(std::string("lalalila5"), MY_NAME, MY_NAME, EXCHANGE_NAME );
       client.sendUnicast(std::string("lalalila6"), MY_NAME, MY_NAME, EXCHANGE_NAME );
       sleep (timeToFlushAllMessages);

       RABBIT_DEBUG ("-------------------------------------------------------");
       RABBIT_DEBUG ("Tester:: Going to send 6 kukuruku messages to NON relevant queue. Should NOT be recieved");
       client.sendUnicast(std::string("kukuriku 1"), OTHER_NAME, MY_NAME, EXCHANGE_NAME );
       client.sendUnicast(std::string("kukuriku 2"), OTHER_NAME, MY_NAME, EXCHANGE_NAME );
       client.sendUnicast(std::string("kukuriku 3"), OTHER_NAME, MY_NAME, EXCHANGE_NAME );
       client.sendUnicast(std::string("kukuriku 4"), OTHER_NAME, MY_NAME, EXCHANGE_NAME );
       client.sendUnicast(std::string("kukuriku 5"), OTHER_NAME, MY_NAME, EXCHANGE_NAME );
       client.sendUnicast(std::string("kukuriku 6"), OTHER_NAME, MY_NAME, EXCHANGE_NAME );
       sleep (timeToFlushAllMessages);

       RABBIT_DEBUG ("-------------------------------------------------------");
       RABBIT_DEBUG ("Tester::  + Bind to the Non relevant queue * Multicast *");
       RABBIT_DEBUG ("Tester:: Going to send bind command");
       client.bindToDestination (OTHER_NAME);
       RABBIT_DEBUG ("Tester:: Going to send mamamia to previous NON relevant queue, after binding to it. Should recieved");
       sleep (timeToBindUnbind);
       client.sendMulticast(std::string("mamamia"), OTHER_NAME, EXCHANGE_NAME );
       sleep (timeToFlushAllMessages);

       RABBIT_DEBUG ("-------------------------------------------------------");
       RABBIT_DEBUG ("Tester:: + unbind from the Non relevant queue ");
       RABBIT_DEBUG ("Tester:: Going to send Unbind command");
       client.unbindFromDestination (OTHER_NAME);
       RABBIT_DEBUG ("Tester:: Going to send kalamari to previously binded queue, after unbinding it. Should NOT recieved");
       sleep (timeToBindUnbind);
       client.sendMulticast(std::string("Kalamari"), OTHER_NAME, EXCHANGE_NAME );
        
       RABBIT_DEBUG ("-------------------------------------------------------");
       RABBIT_DEBUG ("Tester::  + Bind to the Non relevant queue * Unicast *");
       RABBIT_DEBUG ("Tester:: Going to send bind command");
       client.bindToSelf (OTHER_NAME);
       RABBIT_DEBUG ("Tester:: Going to send mamamia to previous NON relevant queue, after binding to it. Should recieved");
       sleep (timeToBindUnbind);
       client.sendUnicast(std::string("mamamia"), OTHER_NAME, MY_NAME, EXCHANGE_NAME );
       sleep (timeToFlushAllMessages);
       RABBIT_DEBUG ("-------------------------------------------------------");
       RABBIT_DEBUG ("Tester:: + unbind from the Non relevant queue ");
       RABBIT_DEBUG ("Tester:: Going to send Unbind command");
       client.unbindFromSelf (OTHER_NAME);
       RABBIT_DEBUG ("Tester:: Going to send kalamari to previously binded queue, after unbinding it. Should NOT recieved");
       sleep (timeToBindUnbind);
       client.sendUnicast(std::string("Kalamari"), OTHER_NAME, MY_NAME, EXCHANGE_NAME );
       sleep (timeToFlushAllMessages);
       sleep (timeToFlushAllMessages);
       return 0;
   }
};

#define SUB_TV(TV1, TV2) (TV1.tv_sec - TV2.tv_sec) * 1000000 + (TV1.tv_usec - TV2.tv_usec)
/*****************************************************************************\
 *  Measure Tester 
\*****************************************************************************/
class MeasureTester 
{
 public:
   MeasureTester ():
       firstReceiveTime ( ),
       lastReceiveTime ( ),
       firstSendTime ( ),
       lastSendTime ( ),
       numOfReceived ( 0 ),
       numOfMessagesToSend ( 1000000 )
    { }
   int operator ()()
   {
       RABBIT_DEBUG("Tester:: Tester Started");
       ConnectionDetails cnd( USER, PASSWORD, RABBIT_IP1, RABBIT_PORT );
       RabbitClient client (cnd, EXCHANGE_NAME, MY_NAME, [ this ] ( std::string o_sender,
                   std::string destination, 
                   DeliveryType deliveryType , 
                   std::string message )->int {
               return onMessageReceive( o_sender, destination, deliveryType, message );
               } ) ;
               client.start();
               while( ! client.connected() )
                   sleep( 1 );
               RABBIT_DEBUG("Tester:: Tester Connected");
               RABBIT_DEBUG("Going to send " << numOfMessagesToSend << " Messages");
               gettimeofday(&firstSendTime, nullptr);
               for (unsigned int leftToSend = numOfMessagesToSend; leftToSend > 0; --leftToSend)
                   client.sendUnicast(std::string("lalalila"), MY_NAME, MY_NAME, EXCHANGE_NAME );
               gettimeofday(&lastSendTime, nullptr);
               for( int iter = 0; iter <= 3600 ; ++iter )
               {
                   sleep(timeToFlushAllMessages);
                   RABBIT_DEBUG("-----------------------------------------");
                   RABBIT_DEBUG(" after "<< ( 1 + iter ) * timeToFlushAllMessages << "secs" );
                   RABBIT_DEBUG("Messages Received  : " << numOfReceived);
                   RABBIT_DEBUG("Sending Time                           (ms) : " << SUB_TV(lastSendTime, firstSendTime )) ;
                   RABBIT_DEBUG("Recieving Time                         (ms) : " << SUB_TV(lastReceiveTime, firstReceiveTime ) );
                   RABBIT_DEBUG("Time from first sent to last revieve   (ms) : " << SUB_TV(lastReceiveTime, firstSendTime)) ;

               }
               return 0;
   }

   int onMessageReceive (std::string, std::string, DeliveryType, std::string)
   {
       if (firstReceiveTime.tv_sec == 0)
           gettimeofday(&firstReceiveTime, nullptr); 
       else
           gettimeofday(&lastReceiveTime, nullptr);
       ++numOfReceived;
       return 0;

   }
 private:
   timeval firstReceiveTime;
   timeval lastReceiveTime;
   timeval firstSendTime;
   timeval lastSendTime;
   unsigned int numOfReceived;
   unsigned int numOfMessagesToSend;
};

/*****************************************************************************\
 *  Repeated bind Tester 
\*****************************************************************************/
class RepeatedBindTester
{
 public:

   int operator ()()
   {
       ConnectionDetails cnd( USER, PASSWORD, RABBIT_IP1, RABBIT_PORT );
       cnd.addAlternateHost("10.10.10.10");
       cnd.addAlternatePort(25);
       RabbitClient client (cnd, EXCHANGE_NAME, MY_NAME, [] ( std::string o_sender, std::string o_destination, DeliveryType, std::string o_message )->int {
               std::cout <<"Free CB:: Received: "<< o_message 
               <<" From : "<< o_sender << std::endl;
               return 0;
               } );
       client.start();
       sleep (timeToConnect);
       while (1)
           BindTester::test(client);
       client.stop(false);
       return 0;
   }
};

/*****************************************************************************\
 *  Continous send Tester 
\*****************************************************************************/
class ContinousSendTester
{
 public:

   int operator ()()
   {
       std::string myID ("MyId");
     ConnectionDetails cnd( USER, PASSWORD, RABBIT_IP1, RABBIT_PORT );
       RabbitClient client (cnd, EXCHANGE_NAME, MY_NAME, [] ( std::string o_sender, std::string o_destination, DeliveryType, std::string o_message )->int {
               std::cout <<"Free CB:: Received: "<< o_message 
                        <<" From : "<< o_sender << std::endl;
               return 0;
               } );
       client.start();
       sleep (timeToConnect);
       RABBIT_DEBUG ( "Tester:: Tester started ");
       RABBIT_DEBUG ("Tester::  + Bind to queue \"kuku\"");
       client.bindToDestination ("kuku");
       sleep (timeToBindUnbind);
       client.sendUnicast(std::string("mamamia"), std::string("kuku"), myID, EXCHANGE_NAME );
       sleep (timeToFlushAllMessages);
       RABBIT_DEBUG("Tester:: Goint to send continous flow of messages to the 2 relevant queues");
       while (1)
       {
           client.sendUnicast(std::string("lalalila"), MY_NAME, MY_NAME, EXCHANGE_NAME );
           client.sendUnicast(std::string("mamamia"), OTHER_NAME, MY_NAME, EXCHANGE_NAME );
           sleep (timeToFlushAllMessages);
           RABBIT_DEBUG ("-------------------------------------------------------");
       }
       client.stop(false);
       return 0;
   }
};

/*****************************************************************************\
 *  Continous send Tester 
\*****************************************************************************/
class ManyBindTester
{
    public:
    int operator ()()
    {
        std::string myID ("MyId");

        ConnectionDetails cnd( USER, PASSWORD, RABBIT_IP1, RABBIT_PORT );
        RabbitClient client (cnd, EXCHANGE_NAME, MY_NAME, [] ( std::string o_sender, std::string o_destination, DeliveryType, std::string o_message )->int {
                std::cout <<"Free CB:: Received: "<< o_message 
                <<" From : "<< o_sender << std::endl;
                return 0;
                } );
        client.start();
        sleep (timeToConnect);
        RABBIT_DEBUG ( "Tester:: Tester started ");
        RABBIT_DEBUG ("Tester::  + Bind to queues ");
        for( int keyIndex = 0; keyIndex < 2000; ++keyIndex )
        {
            std::string key = std::string( "key" ) + boost::lexical_cast< std::string >( keyIndex );
            client.bindToDestination ( key );
        }
        sleep (timeToBindUnbind);
        RABBIT_DEBUG ("Tester::  + sending messages to queues ");
        for( int keyIndex = 0; keyIndex < 2000; ++keyIndex )
        {
            std::string key = std::string( "key" ) + boost::lexical_cast< std::string >( keyIndex );
            client.sendMulticast(std::string("mamamia"), key, EXCHANGE_NAME );
        }
        sleep (timeToFlushAllMessages);
        sleep (timeToFlushAllMessages);
        return 0;
    }
};


/*****************************************************************************\
 * Main
\*****************************************************************************/
int main ()
{
    BindTester bindTester;
    MeasureTester measureTester;
    RepeatedBindTester repeatediBindTester;
    ContinousSendTester continousTester;
    ManyBindTester manyBindTester;

    ( void ) bindTester;
    ( void ) measureTester;
    ( void ) repeatediBindTester;
    ( void ) continousTester;
    ( void ) manyBindTester;

    std::thread testerThread( std::bind( & BindTester::operator(), & bindTester) );
//    std::thread testerThread( std::bind( & MeasureTester::operator(), & measureTester ) );
//    std::thread testerThread( std::bind( & RepeatedBindTester::operator(), & repeatediBindTester ) );
//    std::thread testerThread( std::bind( & ContinousSendTester::operator(), & continousTester ) );
//    std::thread testerThread( std::bind( & ManyBindTester::operator(), & manyBindTester ) );

    testerThread.join();
    RABBIT_DEBUG ("Tester:: Test finished");
}
