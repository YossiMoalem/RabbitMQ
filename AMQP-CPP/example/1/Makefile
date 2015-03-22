CPP				= g++-4.8
CPPFLAGS		= -Wall -c -I. -O0 -flto -pthread -std=c++11 -g -I..  -gdwarf-2
LD				= g++-4.8
LDFLAGS			= -L. -lamqpcpp -lev -pthread -std=c++11
RESULT			= a.out
SOURCES			= $(wildcard *.cpp)
OBJECTS			= $(SOURCES:%.cpp=%.o)

all:	${OBJECTS} ${RESULT}

${RESULT}: ${OBJECTS}
	${LD} -o $@ ${OBJECTS} ${LDFLAGS}

clean:
	${RM} *.obj *~* ${OBJECTS} ${RESULT}

${OBJECTS}:
	${CPP} ${CPPFLAGS} -o $@ ${@:%.o=%.cpp}
