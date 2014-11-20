INCLUDE = -I/usr/local/include -I../../../include -Iinclude/

CXX      = g++
CFLAGS   = -Wall -g -O2 $(INCLUDE) -MD -g
CPPFLAGS = $(CFLAGS) -std=c++0x

LIBS     = -L/usr/local/lib
LIBS    += -lrabbitmq
LIBS    += -lamqpcpp
LIBS    += -lboost_thread


SOURCES  = producer.cpp
SOURCES  += ../../../src/AMQP.cpp ../../../src/AMQPBase.cpp ../../../src/AMQPException.cpp ../../../src/AMQPMessage.cpp ../../../src/AMQPExchange.cpp ../../../src/AMQPQueue.cpp
EXFILES  = producer.cpp
EXAMPLES = $(EXFILES:.cpp=)
OBJECTS  = $(SOURCES:.cpp=.o)
	        
PROGS   = producer
		        
#all: $(PROGS)

LIBNAME  = amqpcpp
LIBFILE  = lib$(LIBNAME).a
all: lib $(EXAMPLES)

lib: $(LIBFILE)

$(LIBFIL+ rcs $@ $(OBJECTS)

$(EXAMPLES): $(EXFILES) $(LIBFILE)
	$(CXX) $(CPPFLAGS) -o $@ $@.cpp $(LIBFILE) $(LIBS)