# Makefile for nuggets game
#
# Team JEN, February 2021

L = lib
S = support

PROG = server
OBJS = server.o
LLIBS = $L/lib.a $S/support.a -lm

# uncomment the following to turn on verbose memory logging
# TESTING=-DMEMTEST

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I$L -I$S
CC = gcc
MAKE = make

$(PROG): $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LLIBS) -o $@

server.o: $S/message.h $S/log.h $L/hashtable.h $L/grid.h

$S/support.a:
	make -C $S support.a

$L/lib.a:
	make -C $L lib.a

.PHONY: test valgrind clean all

all:
	$(MAKE) -C lib
	$(MAKE) -C support
	make

clean:
	make -C $S clean
	make -C $L clean
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f $(PROG)
	rm -f core
	rm -f vgcore*
