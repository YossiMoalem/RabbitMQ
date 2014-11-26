#include "simpleClient.h"

//TODO: find a way to remove this
#include <AMQPcpp.h>

#include <sys/time.h>

static const unsigned int timeToConnect = 3;
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
       connectionDetails cnd("adam", "adam", "rabbit1", 5672);
       simpleClient client (cnd, "EXC1", "USR1", [] ( AMQPMessage * message )->int {
               uint32_t messageLength = 0;
               const char * msg = message->getMessage(&messageLength);
               std::string message_text;
               message_text.assign(msg, messageLength);
               std::cout <<"Free CB:: Recieved: "<<message_text << std::endl;
               return 0;
               } );
       client.start();
       sleep (timeToConnect);
       test(client);
       client.stop(false);
       return 0;
   }

   static int test (simpleClient& client)
   {
       RABBIT_DEBUG ( "Tester:: Tester started ");
       RABBIT_DEBUG ("-------------------------------------------------------");
       RABBIT_DEBUG ("Tester::  + Send message to relevant/nonrelevane queue:");
       RABBIT_DEBUG ("Tester:: Going to send lalalila to the relevant queue. Shold be recieved");
       client.sendUnicast(std::string("lalalila"), std::string("USR1"));
       RABBIT_DEBUG ("Tester:: Going to send kukuruku to NON relevant queue. Shold NOT be recieved");
       client.sendUnicast(std::string("kukuriku "), std::string("kuku"));
       sleep (timeToFlushAllMessages);
       RABBIT_DEBUG ("-------------------------------------------------------");
       RABBIT_DEBUG ("Tester::  + Bind to the Non relevant queue ");
       RABBIT_DEBUG ("Tester:: Going to send bind command");
       client.bindToDestination ("kuku");
       RABBIT_DEBUG ("Tester:: Going to send mamamia to previous NON relevant queue, after binding to it. Shold recieved");
       sleep (timeToBindUnbind);
       client.sendMulticast(std::string("mamamia"), std::string("kuku"));
       sleep (timeToFlushAllMessages);
       RABBIT_DEBUG ("-------------------------------------------------------");
       RABBIT_DEBUG ("Tester:: + unbind from the Non relevant queue ");
       RABBIT_DEBUG ("Tester:: Going to send Unbind command");
       client.unbindFromDestination ("kuku");
       RABBIT_DEBUG ("Tester:: Going to send kalamari to previously binded queue, after unbinding it. Shold NOT recieved");
       sleep (timeToBindUnbind);
       client.sendMulticast(std::string("Kalamari"), std::string("kuku"));
       sleep (timeToFlushAllMessages);

       return 0;
   }
};

#define SUB_TV(TV1, TV2) (TV1.tv_sec - TV2.tv_sec) * 1000000 + (TV1.tv_usec - TV2.tv_usec)
/*****************************************************************************\
 *  Measure Tester 
\*****************************************************************************/
static timeval firstRecieveTime = {0, 0};
static timeval lastRecieveTime = {0, 0};
static timeval firstSendTime = {0, 0};
static timeval lastSendTime = {0, 0};
unsigned int numOfRecieved = 0;
unsigned int numOfMessagesToSend = 1000000;

class MeasureTester
{
 public:
   int operator ()()
   {

       connectionDetails cnd("adam", "adam", "rabbit1", 5672);
       simpleClient client (cnd, "EXC1", "USR1", [] ( AMQPMessage * message )->int {
               if (firstRecieveTime.tv_sec == 0)
                    gettimeofday(&firstRecieveTime, nullptr); 
                else
                    gettimeofday(&lastRecieveTime, nullptr);
                ++numOfRecieved;
                return 0;
               } );
       client.start();
       sleep(timeToConnect);
       RABBIT_DEBUG("Tester:: Tester Started");
       RABBIT_DEBUG("Going to send " << numOfMessagesToSend << " Messages");
       gettimeofday(&firstSendTime, nullptr);
       for (unsigned int leftToSend = numOfMessagesToSend; leftToSend > 0; --leftToSend)
           client.sendUnicast(std::string("lalalila"), std::string("USR1"));
       gettimeofday(&lastSendTime, nullptr);
       sleep(timeToFlushAllMessages);
       RABBIT_DEBUG("-----------------------------------------");
       RABBIT_DEBUG("Messages Recieved  : " << numOfRecieved);
       RABBIT_DEBUG("Sending Time                           (ms) : " << SUB_TV(lastSendTime, firstSendTime )) ;
       RABBIT_DEBUG("Recieving Time                         (ms) : " << SUB_TV(lastRecieveTime, firstRecieveTime ) );
       RABBIT_DEBUG("Time from first sent to last revieve   (ms) : " << SUB_TV(lastRecieveTime, firstSendTime)) ;

       return 0;
   }
};

/*****************************************************************************\
 *  Repeated bind Tester 
\*****************************************************************************/
class RepeatedBindTester
{
 public:

   int operator ()()
   {
       connectionDetails cnd("adam", "adam", "rabbit1", 5672);
       simpleClient client (cnd, "EXC1", "USR1", [] ( AMQPMessage * message )->int {
               uint32_t messageLength = 0;
               const char * msg = message->getMessage(&messageLength);
               std::string message_text;
               message_text.assign(msg, messageLength);
               std::cout <<"Free CB:: Recieved: "<<message_text << std::endl;
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

class ContinousSendTester
{
 public:

   int operator ()()
   {
       connectionDetails cnd("adam", "adam", "rabbit1", 5672);
       simpleClient client (cnd, "EXC1", "USR1", [] ( AMQPMessage * message )->int {
               uint32_t messageLength = 0;
               const char * msg = message->getMessage(&messageLength);
               std::string message_text;
               message_text.assign(msg, messageLength);
               std::cout <<"Free CB:: Recieved: "<<message_text << std::endl;
               return 0;
               } );
       client.start();
       sleep (timeToConnect);
       RABBIT_DEBUG ( "Tester:: Tester started ");
       RABBIT_DEBUG ("Tester::  + Bind to queue \"kuku\"");
       client.bindToDestination ("kuku");
       sleep (timeToBindUnbind);
       client.sendMulticast(std::string("mamamia"), std::string("kuku"));
       sleep (timeToFlushAllMessages);
       RABBIT_DEBUG("Tester:: Goint to send continous flow of messages to the 2 relevant queues");
       while (1)
       {
           client.sendUnicast(std::string("lalalila"), std::string("USR1"));
           client.sendMulticast(std::string("mamamia"), std::string("kuku"));
           sleep (timeToFlushAllMessages);
           RABBIT_DEBUG ("-------------------------------------------------------");
       }
       client.stop(false);
       return 0;
   }

};

/*****************************************************************************\
 * Main
\*****************************************************************************/
int main ()
{
  //BindTester tester;
  //MeasureTester tester;
  //RepeatedBindTester tester;
  ContinousSendTester tester;
  boost::thread testerThread(tester);

  testerThread.join();

}
