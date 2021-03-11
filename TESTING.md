# Nuggets: Testing Information

This document describes how we tested our modules and programs.

### server.c

We manually tested server.c by running client side commands and documenting the output as log files within our *log* subdirectory.

For each item in the following list, a separate log file is created with a relative path name of `log/serverx.log` where **x** is the respective number in the list. *(i.e. the log file produced from the first test in the list below is found in `log/server1.log`)*

Details on how each test was conducted is provided. We ran all tests with the same seed number (1) to ensure consistent testing.

Assumptions:
- The client only quits by pressing Q

#### 1. Wrong number of arguments
Server:
```bash
./server 2> log/server1.log
```

#### 2. Non-existent map
Server:
```bash
./server 2> log/server2.log maps/dummy.txt 1
```

#### 3. Player name length too long (checking that server truncates the name)
Server:
```bash
./server 2> log/server3.log maps/main.txt 1
```
Then initialize a players using the provided `./player` program.
We used a player with name of length 52 (abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz) like follows:
```bash
./player 2> log/player3.log localhost portnumber abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz
```

#### 4. Adding more than max number of players (adding a 27th player)
Server:
```bash
./server 2> log/server4.log maps/main.txt 1
```
Then initialize 27 players using the provided `./player` program. Check that a 27th player is unable to join the game.

#### 5. Adding two players then a players quit (check that their symbol was removed from map display)
Server:
```bash
./server 2> log/server5.log maps/main.txt 1
```
Then initialize two players using the provided `./player` program. Have one of them quit by pushing the "Q" keyboard command and ensure that the player's symbol was removed from the game play.

#### 6. Adding two spectators and checking that spectator can quit
Server:
```bash
./server 2> log/server6.log maps/main.txt 1
```
Then initialize a spectator using the provided `./player` program by not providing a name. Initialize a second spectator in the same manner. Check that the original spectator was replaced with the new spectator.

Then, push the "Q" key command to exit out of the new spectator and ensure that they were removed from watching the game.

#### 7. Map where number of free room spots is < min number of gold piles
Server:
```bash
./server 2> log/server7.log maps/toosmall.txt 1
```

#### 8. Adding a player directly to a gold piece and immediately collecting gold (as no free room spots)
Server:
```bash
./server 2> log/server8.log maps/kindasmall.txt 1
```

#### 9. Swapping two players
Server:
```bash
./server 2> log/server9.log maps/main.txt 1
```
Then initialize two players using the provided `./player` program. Have one player move onto the position of the other player. Ensure that the two players swap positions correctly and that this is correctly displayed on the grid.

#### 10. Seeing a gold pile and then moving outside its visibility
Server:
```bash
./server 2> log/server10.log maps/main.txt 1
```
Then initialize a player using the provided `./player` program. Have the player see a pile a gold but not pick it up. Ensure that once the player moves out of the visibility of the gold behind it left behind, that they can no longer see the gold pile (should be changed to a regular room spot).

#### 11. collecting all the gold
Server:
```bash
./server 2> log/server11.log maps/small.txt 1
```
Then initialize a player using the provided `./player` program. Have the player collect all the gold piles. Ensure that the game concludes by sending the player the game result message.

### lib/grid.c

We created a testing program for the ***grid*** module. The testing program is located in the *lib* subdirectory, in a program named `gridtest`.

See the `Makefile` in the *lib* subdirectory for the compilation.

To compile, head over to the *lib* subdirectory and call:

	make gridtest

Compiling the test program will also immediately **run the program** following compilation. However, if you would like to *run the test again* after compilation, call the following in the command line:

```bash
./gridtest
```

The test program will run a test case, and print to stdout if the test case passed successfully.
On any error, we print a failure message to stdout as well.
Upon the conclusion of running all test cases, the program will also print to stdout the number of failed cases.

The test cases we tested were:
  * Initializing a new ***grid*** to represent the game map *grid_struct_new*
	* Loading the ***grid*** with a sample map file by calling *grid_load*
	* Testing all of the getter functions by comparing the output value with the expected value
	* Testing all of the setter functions by setting parameters to new values, then using the getter functions to check if the parameters have been updated to our desired values
	* Deleting an existing ***grid*** and the memory allocated for it with *grid_delete*
	* Passing a NULL ***grid*** to getter functions and ensuring correct values are returned
	* Passing a NULL ***grid*** to setter functions and ensuring no errors occur

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
