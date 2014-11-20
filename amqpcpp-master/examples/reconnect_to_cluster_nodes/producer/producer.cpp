#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <boost/thread/thread.hpp>
#include "AMQPcpp.h"

#include "SynchronizedQueue.h"

char* brokers[1] = {"rabbit1"};
int ports[1] = {5672};
bool isConnected = false;
std::string username = "adam";
std::string password = "adam";
std::string exchangeName = "exchange1";
std::string exchangeType = "direct";
std::string routing_key = "queue1";

typedef std::string  protocol_t;
SynchronizedQueue<protocol_t> m_q[1];


int reconnects = 0;
void publish() {
    try {
        reconnects++;
        std::cout << "Connecting:" << reconnects << "..." << std::endl;

        srand((unsigned)time(0));
        std::stringstream ss;
        int selected_port = ports[rand() % (sizeof(ports)/sizeof(*ports))] ;
        std::string selected_node = brokers[rand() % (sizeof(brokers)/sizeof(*brokers))] ;
        ss << username << ":" << password << "@" << selected_node << ":" << selected_port;
        AMQP amqp(ss.str());

        isConnected = true;
        AMQPExchange * ex = amqp.createExchange(exchangeName);
        ex->Declare(exchangeName, exchangeType);

        std::cout << "Connected." << std::endl;
        reconnects = 0;

        ex->setHeader("Content-type", "text/text");
        ex->setHeader("Content-encoding", "UTF-8");
        ex->setHeader("Delivery-mode", 1);


        int counter = 0;
        ISynchronizedQueue<protocol_t>* pQ=(ISynchronizedQueue<protocol_t>*)m_q;
        while(true)
        {
            boost::this_thread::sleep( boost::posix_time::milliseconds(50) );

            protocol_t  protocol;
            while (pQ->get(protocol)) {
                ex->Publish(protocol, routing_key);
                counter++;
                std::cout << protocol << std::endl;
                /*
                if (0 == counter % 1000) {
                    cout << protocol << endl;
                }
                */
            }
        }

    } catch (AMQPException e) {
        isConnected = false;
        std::cout << e.getMessage() << std::endl;
        boost::this_thread::sleep( boost::posix_time::milliseconds(3000) );
        publish();
    }
}


std::string msg;
int counter = 0;
void produce() {
    ISynchronizedQueue<protocol_t>* pQ = (ISynchronizedQueue<protocol_t>*)m_q;
    while(true)
    {
        counter++;
        std::stringstream ss;
        ss << msg << ":" << counter;
        pQ->add(ss.str());
        boost::this_thread::sleep( boost::posix_time::milliseconds(1000) );
    }
}


int main (int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./producer \"text to send\"\n");
        return 1;
    }

    msg = std::string(argv[1]);

    boost::thread_group threads;

    threads.create_thread(produce);
    threads.create_thread(publish);

    // Wait for Threads to finish.
    threads.join_all();

    return 0;
}
