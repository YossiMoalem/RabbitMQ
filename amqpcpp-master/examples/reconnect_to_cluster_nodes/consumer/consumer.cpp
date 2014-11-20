//TODO: allow mutiple bindings with one msg
//TODO: use single connection?
//TODO: make sure we dont have exception when binding to the same queue again (and that we dont get duplicate msgs). or unbinding twice.
//TODO: fix consumer not detecting that he is offline
//TODO: add timeout to connect, when trying to connect to an offline MB

#include <unordered_set>
#include <atomic>
#include <boost/thread/thread.hpp>
#include <AMQPcpp.h>
#include "SynchronizedQueue.h"

#define BIND_PREFIX "BIND::"
#define UNBIND_PREFIX "UNBIND::"

static const char* const brokers[1] = {"rabbit1"};
static int ports[1] = {5672};
static std::atomic<bool> isConsumerConnected(false);
static std::atomic<bool> isPublisherConnected(false);
static const std::string username = "adam";
static const std::string password = "adam";
static const std::string queueName = "queue1";
static const std::string exchangeName = "exchange1";
static const std::string exchangeType = "direct";
static const std::string routing_key = queueName;
static std::unordered_set<std::string> subscriptionsList;
static std::string msg;

class MyMessage {
public:
    MyMessage()
    { };

    MyMessage(std::string text, std::string destination, std::string type)
            : message_text(text), message_destination(destination), message_type(type)
    { };

    std::string message_text;
    std::string message_destination;
    std::string message_type;

};

typedef MyMessage protocol;
SynchronizedQueue<protocol> messageQueueReceived[1];
SynchronizedQueue<protocol> messageQueueToSend[1];


void send(std::string message, std::string destination, std::string type) {
    if (isPublisherConnected) {
        SynchronizedQueue<protocol> & pQ = messageQueueToSend[0];
        std::stringstream ss;
//        ss << message << ":" << counter2;
//        pQ->add(MyMessage(ss.str(), destination, type));
        pQ.add(protocol(message, destination, type));
    }
}

void send_publish(std::string message, std::string destination) {
    send(message, destination, "publish");
}

void send_unicast(std::string message, std::string destination) {
    send(message, destination, "unicast");
}

void subscribe(std::string routing_key, bool used_by_rebind) {
    if (!used_by_rebind)
    {
        subscriptionsList.insert(routing_key);
    }
    send( BIND_PREFIX + routing_key, queueName, "unicast");
}

void unsubscribe(std::string routing_key) {
    subscriptionsList.erase(routing_key);
    if (isConsumerConnected)
    {
        send( UNBIND_PREFIX +routing_key, queueName, "unicast");
    }
}

void rebind_when_connecting () {
    std::cout << "Rebind queue to keys" << std::endl;
    auto lastSubscription = subscriptionsList.end();
    for (auto i = subscriptionsList.begin(); i != lastSubscription ; ++i) {
        subscribe((*i), true);
    }
}

int onCancel(AMQPMessage * message ) {
    std::cout << "cancel tag="<< message->getDeliveryTag() << std::endl;
    return 0;
}

int onMessage( AMQPMessage * message  ) {
    static int i = 0;
    i++;
    uint32_t messageLength = 0;

    // if msg is bind/unbind, do it
    const char * msg = message->getMessage(&messageLength);
    std::string message_text;
    message_text.assign(msg, messageLength);
//    std::cout << message_text << " -- here" << std::endl;

    if ( isConsumerConnected )
    {
        //TODO: I asume the message length is greater than BIND_PREFIX and UNBIND_PREFIX
        if (message_text.compare (0, strlen( BIND_PREFIX ), BIND_PREFIX ) == 0 ) //bind - BIND::DESTINATION
        {
            message->getQueue()->Bind( exchangeName, message_text.substr(strlen( BIND_PREFIX ) ) );
            return 0;
        }
        if ( message_text.compare (0, strlen( UNBIND_PREFIX ), UNBIND_PREFIX ) == 0 ) //unbind - UNBIND::DESTINATION
        {
            message->getQueue()->unBind( exchangeName, message_text.substr(strlen( UNBIND_PREFIX ) ) );
            return 0;
        }
    }

    SynchronizedQueue<protocol> & pQ = messageQueueReceived[ 0 ];
    pQ.add(protocol(message_text.c_str(), message->getRoutingKey(), "should_be_unicast_or_publish"));
//    pQ->add(protocol(message->getMessage(&j), message->getRoutingKey(), "should_be_unicast_or_publish"));

    //if (0 == i % 1000 && message_txt)
    //    cout << message_txt << endl;

    //cout << "#" << i << " tag="<< message->getDeliveryTag()
    //     << " content-type:"<< message->getHeader("Content-type") ;
    //cout << " encoding:"<< message->getHeader("Content-encoding")
    //     << " mode="<<message->getHeader("Delivery-mode")<<endl;

    //if (i > 10) {
    //    AMQPQueue * q = message->getQueue();
    //    q->Cancel( message->getConsumerTag() );
    //}

    return 0;
};

void handle() {
    SynchronizedQueue<protocol> & pQ = messageQueueReceived[ 0 ];
    while(true)
    {
        boost::this_thread::sleep( boost::posix_time::milliseconds(50) );

        protocol  protocol;
        while (pQ.get(protocol)) {
            cout << "Received message: "<< protocol.message_text << " sent to: "  << protocol.message_destination << endl;
        }
    }
}

static std::string getConnectionString ()
{
        srand((unsigned)time(0));
        int selected_port = ports[rand() % (sizeof(ports)/sizeof(*ports))] ;
        std::string selected_node = brokers[rand() % (sizeof(brokers)/sizeof(*brokers))] ;
        std::stringstream ss;
        ss << username << ":" << password << "@" << selected_node << ":" << selected_port;
        return ss.str();
}

void consume() {
    int reconnectAttemptsConsumer = 0;
    static const std::string consumer_tag = "hello-consumer";
    boost::thread_group threadsForRebind;
    for (;;)
    {
        reconnectAttemptsConsumer++;
        std::string connectionString = getConnectionString();
        std::cout << "Connecting:" << reconnectAttemptsConsumer << "..." << connectionString << std::endl;

        try {
          AMQP amqp(connectionString);
          AMQPExchange * ex = amqp.createExchange(exchangeName);
          ex->Declare(exchangeName, exchangeType);


          AMQPQueue * queue = amqp.createQueue(queueName); 
          queue->Declare(queueName); // if we declare with string it will mark queue as auto-delete
          queue->Bind( exchangeName, routing_key);
          std::cout << "Connected." << std::endl;
          reconnectAttemptsConsumer = 0;

          queue->setConsumerTag(consumer_tag);
          queue->addEvent(AMQP_MESSAGE, onMessage );
          queue->addEvent(AMQP_CANCEL, onCancel );
          isConsumerConnected = true;
          threadsForRebind.create_thread(rebind_when_connecting);
          // queue->Consume();
          queue->Consume(AMQP_NOACK);
        }

        catch ( AMQPException e )
        {
            std::cout << e.getMessage() << std::endl;
        }
        isConsumerConnected = false;
        std::cout << "Disconnected!!" << std::endl;
        boost::this_thread::sleep( boost::posix_time::milliseconds(3000) );
    }
}


void publish() {
  int reconnectAttemptsPublisher = 0;
  for (;;)
  {
    try {
      reconnectAttemptsPublisher++;
      std::string connectionString = getConnectionString();

      std::cout << "Connecting:" << reconnectAttemptsPublisher << "..." << connectionString << std::endl;
      AMQP amqp(connectionString);

      isPublisherConnected = true;
      AMQPExchange * ex = amqp.createExchange(exchangeName);
      ex->Declare(exchangeName, exchangeType);

      std::cout << "Connected." << std::endl;
      reconnectAttemptsPublisher = 0;

      ex->setHeader("Content-type", "text/text");
      ex->setHeader("Content-encoding", "UTF-8");
      ex->setHeader("Delivery-mode", 1);


      SynchronizedQueue<protocol> & pQ = messageQueueToSend[ 0 ];
      protocol  protocol;
      // we dont want to empty queue because we will want to send all bind/unbind that were added using rebind
      // while (pQ->get(protocol)) { } //empty the messageQueue if we have any leftovers
      while(true)
      {
        boost::this_thread::sleep( boost::posix_time::milliseconds(50) );
        while (pQ.get(protocol)) {
          ex->Publish(protocol.message_text, protocol.message_destination);
          std::cout << "sending message: " <<protocol.message_text << std::endl;
        }
      }
    } catch (AMQPException e) {
      isPublisherConnected = false;
      std::cout << e.getMessage() << std::endl;
      boost::this_thread::sleep( boost::posix_time::milliseconds(3000) );
    }
  }
}


void produce() {
    SynchronizedQueue<protocol> & pQ = messageQueueToSend[ 0 ];
    static int counter2 = 0;
    while(true)
    {
        if (isPublisherConnected)
        {
            counter2++;
            std::stringstream ss;
            ss << msg << ":" << counter2;
            pQ.add(protocol(ss.str(), queueName, "type"));
            boost::this_thread::sleep( boost::posix_time::milliseconds(1000) );
        }
    }
}


int main(int argc, char** argv) {

    if (argc < 2) {
        fprintf(stderr, "Usage: ./consumer \"text to send\"\n");
        return 1;
    }
    msg = std::string(argv[1]);

    boost::thread_group threads;
//    threads.create_thread(produce); // create msgs in queue
    threads.create_thread(consume); // listen for incoming msgs
    threads.create_thread(handle);  // handle consumed msgs
    threads.create_thread( publish ); // publish them

    boost::this_thread::sleep( boost::posix_time::milliseconds(3500) );
    send(msg, queueName, "unicast");

    boost::this_thread::sleep( boost::posix_time::milliseconds(3000) );
    subscribe("destination", false);

    boost::this_thread::sleep( boost::posix_time::milliseconds(1000) );
    send("you should see me", "destination", "unicast");

    boost::this_thread::sleep( boost::posix_time::milliseconds(1000) );
    unsubscribe("destination");

    boost::this_thread::sleep( boost::posix_time::milliseconds(1000) );
    send("you should not receive me........", "destination", "unicast");


//    subscribe("1");
//    send(msg, "1", "test");
//    subscribe("2");
//    subscribe("3");
//    std::cout << "subscriptionsList contains:";
//    for ( const std::string& x: subscriptionsList ) std::cout << " " << x;
//    std::cout << std::endl;
//    unsubscribe("2");
//    for ( const std::string& x: subscriptionsList ) std::cout << " " << x;
//    std::cout << std::endl;

    // Wait for Threads to finish.
    threads.join_all();

    return 0;
}
#undef BIND_PREFIX 
#undef UNBIND_PREFIX 
