CC=g++
FLAGS=-g -Wall -std=c++98
BINNAME=refstats
OBJECTS=monitor.o helper.o

all: ${OBJECTS}
	${CC} ${FLAGS} main.cc ${OBJECTS} -o ${BINNAME} -pthread

test: monitor.o
	${CC} ${FLAGS} test.cc monitor.o -o testProg -pthread
	
monitor.o:
	${CC} ${FLAGS} monitor.cc -o monitor.o -c
	
helper.o:
	${CC} ${FLAGS} helper.cc -o helper.o -c
	
clean:
	rm ${BINNAME} *.o
	
tar:
	tar zcvf monitor.tgz *
	