# Makefile for CS50 nuggets library
#   Builds and compiles all these utility functions into a library.
#
# Team JEN, Winter 2021

# object files, and the target library

OBJS = hashtable.o memory.o set.o jhash.o file.o grid.o server_player.o $L/message.h
LIB = lib.a
L = ../support

# add -DNOSLEEP to disable the automatic sleep after web-page fetches
#               (students, please do not use -DNOSLEEP!)
# add -DMEMTEST for memory tracking report in indexer
# (and run `make clean; make` whenever you change this)
FLAGS = # -DMEMTEST  # -DNOSLEEP

CFLAGS = -Wall -pedantic -std=c11 -ggdb -I$L $(FLAGS)
CC = gcc
MAKE = make

# Build the library by archiving object files
$(LIB): $(OBJS)
	ar cr $(LIB) $(OBJS)

# to test the server_player
server_playertest: $(OBJS) server_playertest.o $L/support.a
	$(CC) $(CFLAGS) $^ -lm $L/support.a -o $@
	./server_playertest

# to test the grid
gridtest: $(OBJS) gridtest.o $L/support.a
	$(CC) $(CFLAGS) $^ -lm $L/support.a -o $@
	./gridtest

$L/support.a:
	make -C $(L) support.a

# Dependencies: object files depend on header files
hashtable.o: hashtable.h set.h jhash.h
jhash.o: jhash.h
memory.o: memory.h
set.o: set.h
file.o: file.h
grid.o: grid.h
server_player.o: server_player.h $L/message.h
server_playertest.o: server_player.h grid.h $L/message.h
gridtest.o: grid.h

.PHONY: clean sourcelist

# list all the sources and docs in this directory.
# (this rule is used only by the Professor in preparing the starter kit)
sourcelist: Makefile *.md *.c *.h
	@echo $^

# clean up after our compilation
clean:
	rm -f $(LIB) *~ *.o
	rm -f core
	rm -rf *~ *.o *.gch *.dSYM
	rm -f *.log
	rm -f server_playertest
	rm -f gridtest
