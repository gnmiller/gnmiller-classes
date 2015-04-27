CC       = c++
FLAGS    = 
CFLAGS   = -g -O2 -Wno-deprecated -Wno-write-strings
DFLAGS   = -DPACKAGE=\"threadsystem\" -DVERSION=\"1.0\" -DPTHREAD=1 -DUNIX_MSG_Q=1 -DSTDC_HEADERS=1
IFLAGS   = -I/local/eit-linux/apps/ThreadMentor/include
TMLIB    = /local/eit-linux/apps/ThreadMentor/Visual/libthreadclass.a
TMLIB_NV    = /local/eit-linux/apps/ThreadMentor/NoVisual/libthreadclass.a

OBJ_FILE = thread.o thread-main.o
EXE_FILE = prog3

prog3		: thread.o thread-main.o
	g++ -o prog3 thread.o thread-main.o ${TMLIB_NV} -lpthread
	
thread.o	:
	g++ ${DFLAGS} ${IFLAGS} ${CFLAGS} -c thread.cpp
	
thread-main.o	:
	g++ ${DFLAGS} ${IFLAGS} ${CFLAGS} -c thread-main.cpp

clean:
	rm -f ${OBJ_FILE} ${EXE_FILE}