CC=gcc
FLAGS=-std=c99

BINNAME=vig
OBJ=vig.o

all: vig.o
	${CC} ${FLAGS} ${OBJ} -o ${BINNAME}

vig.o:
	${CC} ${FLAGS} vig.c -c -o vig.o

clean:
	rm ${OBJ}
	rm vig
