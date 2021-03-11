# Nuggets: Testing Information

This document describes how we tested our modules and programs.

### server.c

We manually tested server.c by running client side commands and documenting the output as log files within our *log* subdirectory. \
For each item in the follow list, a separate log file is created with a relative path name of log/serverx.log where x is the respected\
number in the list i.e. the first item on the list is found in log/server1.log. All tests were run with the same seed (1) except\
for the one test for no seed provided. The server input is represented within the parentheses.

1. wrong number of arguments (./server 2> log/server1.log)
2. non-existent map (./server 2> log/server2.log maps/dummy.txt 1)
3. name length too long (abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz (length = 52))
4. Adding 27 players (./server 2> log/server4.log maps/main.txt 1)
5. Adding two players then removing a player
6. Adding two spectators
7. Map where number of free room spots is < min number of gold piles
8. Adding a player directly to a gold piece and immediately collecting gold (as no free room spots)
9. swapping two players
10. seeing a gold pile and then moving outside its visibility
11. collecting all the gold

Assumptions:
- The client only quits by pressing Q
- If there are no free room spots available and there

### lib/grid.c


### lib/server_player.c

We created a testing program for the ***server_player*** module. The testing program is located in the *lib* subdirectory, in a program named `server_playertest`.

See the `Makefile` in the *lib* subdirectory for the compilation.

To compile, head over to the *lib* subdirectory and call:

	make server_playertest

Compiling the test program will also immediately **run the program** following compilation. However, if you would like to *run the test again* after compilation, call the following in the command line:

```bash
./server_playertest
```

The test program will run a test case, and print to stdout if the test case passed successfully.
On any error, we print a failure message to stdout as well.
Upon the conclusion of running all test cases, the program will also print to stdout the number of failed cases.

The test cases we tested were:
  * Initializing a new ***server_player*** to represent a player with *server_player_new*
	* Initializing a new ***server_player*** to represent a spectator with *server_player_new*
	* Testing all of the getter functions by comparing the output value with the expected value
	* Testing all of the setter functions by setting parameters to new values, then using the getter functions to check if the parameters have been updated to our desired values
	* Deleting an existing ***server_player*** that represents a player by calling *server_player_delete*
	* Deleting an existing ***server_player*** that represents a spectator by calling *server_spectator_delete*
	* Passing a NULL ***server_player*** to getter functions and ensuring correct values are returned
	* Passing a NULL ***server_player*** to setter functions and ensuring no errors occur
