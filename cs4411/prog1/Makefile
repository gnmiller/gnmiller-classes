######################################################################
# CS4411 :: Introduction to Operating Systems.  (Spring 2015)        #
# Programming Assignment I : File erasure using system call wrappers #
#                                                                    # 
# Simple Makefile Template			    	 	     				#
#                                                                    # 
# Requires small changes to OBJ definition for project.              #
# Currently set up to compile using rename.c example.                #
######################################################################

#the compiler to be used for this assignment.
CC = gcc
RM = rm

#Flags for GCC. uncomment the second line to define the NOCALL flag
# Assume here that command "make 'CFLAGS=-DNOCALL'" will be used to 
# add definition of NOCALL for testing purposes.  Following statement 
# ensures we always get -Wall -g 
# (See http://www.gnu.org/software/make/manual/make.html#Override-Directive)

override CFLAGS += -Wall  -g 

#
# rename is not part of this assignment.  May be useful for testing your
# understanding before you start writing code
 
#OBJS = ftruncate.o truncate.o unlink.o rename.o
OBJS = truncate.o ftruncate.o unlink.o

# Example test program is tstRename.  You will have your own test program(s)
all: ${OBJS} libWrapper.so tstProg

tstProg: tstProg.o
	gcc -L/home/capus07/gnmiller/cs4411/prog1 -o tstProg tstProg.o  libWrapper.so
	
libWrapper.so: ${OBJS}
	${RM} -f $@ 
	gcc -shared -o $@ ${OBJS}

ftruncate.o: ftruncate.c
	${CC} -fPIC ${CFLAGS} -c $< 

truncate.o: truncate.c
	${CC} -fPIC ${CFLAGS} -c $< 

unlink.o: unlink.c
	${CC} -fPIC ${CFLAGS} -c $< 
	
tstProg.o: tstProg.c
	${CC} ${CFLAGS} -c $<

#Clean all the object and executable files
clean:
	rm -f  *.o tstProg
	rm -f ${LIB} 


####################################################################
