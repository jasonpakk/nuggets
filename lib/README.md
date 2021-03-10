# lib library

This library contains modules used by the server in the CS50 Nuggets project.

## 'file' module

Provides utility functions to read a word, line, or an entire file
See `file.h` for interface details.

## 'grid' module

Used by server to store the contents of the game map in a 2D array.
See `grid.h` for interface details.

## 'hashtable' module

Provides a set of (key,item) pairs.
See `hashtable.h` for interface details.

## 'jhash' module

Provides the Jenkins Hash. Used by the hashtable to map from string to integer.
See `jhash.h` for interface details.

## 'memory' module

Provides functions to allocate and free memory.
See `memory.h` for interface details.

## 'server_player' module

Provides a module representing each player in the game to be used by the server
See `server_player.h` for interface details.

## 'set' module

Provides an unordered collection of (key,item) pairs.
See `set.h` for interface details.

## compiling

To compile,

	make lib.a

To clean,

	make clean

## using

In a typical use, assume this library is a subdirectory named `lib`, within a directory where some main program is located.
The Makefile of that main directory might then include content like this:

```make
S = lib
CFLAGS = ... -I$S
LLIBS = $S/lib.a
LIBS =
...
program.o: ... $S/hashtable.h $S/grid.h ...
program: program.o $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@
...
$S/lib.a:
	make -C $S lib.a

clean:
	make -C $S clean
	...
```

This approach allows the main program to be built (or cleaned) while automatically building (cleaning) the lib library as needed.
