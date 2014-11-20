#include "simpleClient.h"

//TODO: find a way to remove this
#include <AMQPcpp.h>

class Tester{

 public:
   Tester(simpleClient& i_client) :
     m_client(i_client)
  {}


   int operator ()()
   {
     static const unsigned int timeToConnect = 3;
     static const unsigned int timeToBindUnbind = 3;
     static const unsigned int timeToFlushAllMessages = 4;

     sleep (timeToConnect);
     RABBIT_DEBUG ( "Tester:: Tester started ");
     RABBIT_DEBUG ("-------------------------------------------------------");
     RABBIT_DEBUG ("Tester::  + Send message to relevant/nonrelevane queue:");
     RABBIT_DEBUG ("Tester:: Going to send lalalila to the relevant queue. Shold be recieved");
     m_client.sendUnicast(std::string("lalalila"), std::string("USR1"));
     RABBIT_DEBUG ("Tester:: Going to send kukuruku to NON relevant queue. Shold NOT be recieved");
     m_client.sendUnicast(std::string("kukuriku "), std::string("kuku"));
     sleep (timeToFlushAllMessages);
     RABBIT_DEBUG ("-------------------------------------------------------");
     RABBIT_DEBUG ("Tester::  + Bind to the Non relevant queue ");
     RABBIT_DEBUG ("Tester:: Going to send bind command");
     m_client.bindToDestination ("kuku");
     RABBIT_DEBUG ("Tester:: Going to send mamamia to previous NON relevant queue, after binding to it. Shold recieved");
     sleep (timeToBindUnbind);
     m_client.sendMulticast(std::string("mamamia"), std::string("kuku"));
     sleep (timeToFlushAllMessages);
     RABBIT_DEBUG ("-------------------------------------------------------");
     RABBIT_DEBUG ("Tester:: + unbind from the Non relevant queue ");
     RABBIT_DEBUG ("Tester:: Going to send Unbind command");
     m_client.unbindFromDestination ("kuku");
     RABBIT_DEBUG ("Tester:: Going to send kalamari to previously binded queue, after unbinding it. Shold NOT recieved");
     sleep (timeToBindUnbind);
     m_client.sendMulticast(std::string("Kalamari"), std::string("kuku"));
     sleep (timeToFlushAllMessages);

     return 0;
   }
 private:
   simpleClient& m_client;
};

int onMsg( AMQPMessage * message  ) 
{
    uint32_t messageLength = 0;
    const char * msg = message->getMessage(&messageLength);
    std::string message_text;
    message_text.assign(msg, messageLength);
    std::cout <<"Free CB:: Recieved: "<<message_text << std::endl;
    return 0;
}

/*****************************************************************************\
 *
\*****************************************************************************/
int main ()
{
  connectionDetails cnd("adam", "adam", "rabbit1", 5672);
  simpleClient client (cnd, "EXC1", "USR1", onMsg);
  Tester tester(client);
  boost::thread testerThread(tester);
  client.start();

}
