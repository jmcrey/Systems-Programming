CC=gcc
CFLAGS=-g -Wall

all:	TTT ttt

TTT:	TTT.o msg.o
	gcc -o TTT TTT.o msg.o

ttt:	ttt.o msg.o
	gcc -o ttt ttt.o msg.o

msg.o:	msg.c common.h

TTT.o:	TTT.c common.h

ttt.o:	ttt.c common.h

clean:
	rm -f *.o ttt TTT serverloc
