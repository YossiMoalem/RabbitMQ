INCLUDE = -I/usr/local/include -I../../../include -Iinclude/

CXX      = g++
CFLAGS   = -Wall -g -O2 $(INCLUDE) -MD -g
#TODO: replace the -R param with variable. Use same one in the c code↲
CPPFLAGS = $(CFLAGS) -std=c++0x -Wl,-rpath,/usr/local/lib/x86_64-linux-gnu

LIBS     = -L/usr/local/lib
LIBS    += -lrabbitmq
#LIBS    += -lamqpcpp
LIBS    += -lboost_thread


SOURCES  = consumer.cpp SynchronizedQueue.cpp
SOURCES  += ../../../src/AMQP.cpp ../../../src/AMQPBase.cpp ../../../src/AMQPException.cpp ../../../src/AMQPMessage.cpp ../../../src/AMQPExchange.cpp ../../../src/AMQPQueue.cpp
EXFILES  = consumer.cpp
EXAMPLES = $(EXFILES:.cpp=)
OBJECTS  = $(SOURCES:.cpp=.o)

PROGS   = consumer SynchronizedQueue

#all: $(PROGS)

LIBNAME  = amqpcpp
LIBFILE  = lib$(LIBNAME).a
all: lib $(EXAMPLES)

lib: $(LIBFILE)

$(LIBFIL+ rcs $@ $(OBJECTS)

$(EXAMPLES): $(EXFILES) $(LIBFILE)
	$(CXX) $(CPPFLAGS) -o $@ $@.cpp $(LIBFILE) $(LIBS)

SynchronizedQueue: SynchronizedQueue.o
	g++ -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJECTS)
	rm consumer
