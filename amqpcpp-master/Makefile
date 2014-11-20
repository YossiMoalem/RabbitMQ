CXX      = g++ -std=c++0x
CFLAGS   = -Wall -pedantic -g
#TODO: replace the -R param with variable. Use same one in the c code
CPPFLAGS = $(CFLAGS) -I/usr/local/include -L/usr/local/lib -Iinclude -Wl,-rpath,../rabbitmq-c-master/

LIBRARIES= rabbitmq
LIBS     = $(addprefix -l,$(LIBRARIES))

LIBNAME  = amqpcpp
LIBFILE  = lib$(LIBNAME).a

SOURCES  = src/AMQP.cpp src/AMQPBase.cpp src/AMQPException.cpp src/AMQPMessage.cpp src/AMQPExchange.cpp src/AMQPQueue.cpp
EXFILES  = example_publish.cpp example_consume.cpp example_get.cpp
EXAMPLES = $(EXFILES:.cpp=)
OBJECTS  = $(SOURCES:.cpp=.o)


all: lib $(EXAMPLES)

lib: $(LIBFILE)

$(LIBFILE): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

$(EXAMPLES): $(addprefix examples/,$(EXFILES)) $(LIBFILE)
	$(CXX) $(CPPFLAGS) -o $@ examples/$@.cpp $(LIBFILE) $(LIBS)

clean:
	rm -f $(OBJECTS) $(EXAMPLES) $(LIBFILE)
