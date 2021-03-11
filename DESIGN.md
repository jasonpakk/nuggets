# Nuggets: Server Design Spec

### Interface

##### Server
The server's interface with the user starts on the command-line; it must have at least two, but can have up to three arguments.

```bash
$ server map.txt [seed]
```

`map.txt`  - map provided  
`seed` - number for random number generator

##### Client
Clients can join this server after it has been created by calling the following in the command-line.

```bash
$ player hostname port [playername]
```
`hostname`  - the name/ip for the server  
`port` - port of the server  
`playername` - join as spectator if not specified, otherwise the player’s name

### Inputs and outputs

##### Server
*Input:* the inputs are command-line parameters and the contents of the map.txt specified by the first command-line parameter. See the User Interface above. Following this, the server then accepts messages from the clients, using this as input to update the game play appropriately.

*Output:* The server sends output messages to the clients on any updates to the grid or the overall gameplay. The server is responsible for sending the proper game map display message to the client based on the visibility of each grid point.

##### Client
*Input:* the initial inputs are command-line parameters used to connect the client with the server; afterwards, the  keyboard presses that correspond to movements of the player are possible inputs.

*Output:* Client will show the room in the grid that the player is in, with points consisting of boundaries and passages, on to an ASCII screen that updates based on the message sent by the server. The room’s visibility depends on the position of the player, and parts of the room may be hidden at the start.

### Functional Decomposition into Modules

We anticipate the following modules or functions:

 1. *main*, which parses arguments and initializes other modules
 2. *maploader*, which loads the map file
 3. *mapinitializer*, which drops the gold piles on random spots of the rooms depending on the seed
 4. *networkinitializer*, which initializes the network and announces the port
 5. *handleMessage*, which reads the message from the client and handles it appropriately
 6. *accept*, which lets players or spectators into the game
 7. *refresh*, which sends the new GOLD count and the proper DISPLAY message to each player

And some helper modules that provide data structures:

  1. ***player*** to store data about each player
  2. ***position*** to store X and Y coordinates
  3. ***hashtable*** to store a set of players
  4. ***set*** is indirectly used by ***hashtable***
  5. ***point*** to store data about each point in the grid
  6. ***message*** to handle client-server communication
  7. ***log*** to form logfiles
  8. ***grid*** to represent the map


### Pseudo Code for logic/algorithmic flow

The server will run as follows:

1. Start from a command line as shown in the User Interface

2. Parse the command line, validate parameters, initialize other modules

3. Load the map at `map.txt` by calling *maploader* and if it fails , take an appropriate action

4. Call *mapinitializer* to initialize the map by dropping the gold piles at random spots using the seed

5. Call *networkinitializer* to initialize the server and announce the port, storing the port in a server file

6. When a message comes in from the client, call *handleMessage* to parse the message

7. If the message is from a new player trying to join the game:

    7.1. If we have reached the maximum number of players, send an error message and do not add this player

    7.2. Otherwise, call *accept* to initialize a new ***player*** structure and add them to the ***hashtable*** of players

    7.3. Send an OK message back to the player with their symbol on the game board

    7.4. Send GRID, GOLD, and DISPLAY messages to the new player

    7.5. Call *refresh* to send GOLD and DISPLAY messages to all other active players/spectator

8. If the message is from a new spectator trying to watch the game:

    8.1. If we have reached the maximum number of spectators, replace the current spectator with the new spectator

    8.2. Call *accept* to initialize a new ***player*** structure for the spectator

    8.3. Send GRID, GOLD, and DISPLAY messages to the new spectator

    8.4. Call *refresh* to send GOLD and DISPLAY messages to all other active players  

9. If the message is a quit message from the client:

    9.1. If the client is a player, remove their symbol from the board and then from playing the game.

    9.2. If the client is a spectator, remove them from watching the game.

    9.3. Call *refesh* to send GOLD and DISPLAY messages to all other active players

10. If the message is a move message from the client:

    10.1. Check if the move is possible, if not, don't move in any direction

    10.2. Update the grid to move the player to their new position

    10.3. Check for gold that was picked up as a result of the move; if gold was picked up, add it to the players purse and decrement the total gold available in the game

    10.4. Call *refresh* to send GOLD and DISPLAY messages to all active players/spectator

11. If there are no more gold piles left to be collected, quit the game for each client by sending them the summary of the game

12. Close the server and delete all modules used for the gameplay


### Dataflow through modules

1. *main* parses the parameters and passes the map file to the *maploader*
2. *maploader* initializes the ***grid*** by reading the map file, and passes this ***grid*** to the *mapinitalizer*
3. *mapinitializer* randomly swaps `.` values in the ***grid*** with gold piles, and then calls *networkinitializer* to start server to client communication
4. *networkinitializer* hosts the server and announces the port, and whenever it receives a message from the client, it passes the message to *handleMessage*
5. *handleMessage* parses the message to determine the action to take:
    * It calls *accept* to initialize a new ***player*** and add them to the ***hashtable*** of players
    * It calls *refresh* to send GOLD and DISPLAY messages to all active players/spectator
6. *refresh* goes through the ***hashtable*** of players and sends the game state to all active ***players***:
    * It sends a GOLD message by accessing how much gold the ***player*** has collected
    * It sends a DISPLAY message by using the ***player***'s ***grid*** to determine visibility

### Major Data Structures

 1. ***player*** to store data about each player client
     * player's name
     * player's symbol on the map
     * amount of gold they have collected
     * player's current ***position***
     * a ***grid*** representing what the player can see
     * whether the player is currently active/playing the game
 2. ***position*** to store X and Y coordinates
 3. ***hashtable*** to store a set of players
 4. ***set*** is indirectly used by hashtable
 5. ***point*** to store data about each point in the grid
     * the character at this grid point
     * amount of gold in the pile (0 if not a gold spot)
     * whether this point is visible from the player's current location
     * whether the player has ever visited this point before
 6. ***message*** to handle client-server communication
 7. ***log*** to form logfiles
 8. ***grid*** to represent the map
     * number of rows
     * number of columns
     * 2D array of ***point*** to represent the game map

### Testing Plan
- *Unit Testing*: small test program to test each module to make sure it does what it’s supposed to do.

  We will need to test the modules that we have created:

    * Unit Test for ***player*** functionality
    * Unit Test for ***grid*** functionality
    * Unit Test for ***point*** functionality
    * Unit Test for ***position*** functionality

  Each unit test will ensure the module functions as intended, as well as check boundary and error cases to ensure the module can recover from such errors gracefully. The unit tests will purposely pass invalid arguments to the module's methods to ensure no errors occur.

  Since the modules ***hashtable*** and ***set*** were provided by the CS 50 Library in the past, and testing was conducted on these data structures in previous labs, we do not run unit tests for these modules in the Nuggets Project.

  Since the modules ***message*** and ***log*** were provided in the starter kit for the Nuggets Project, we do not run our own unit tests for these modules in the Nuggets Project. An example unit test is already provided in the ***message*** module.  

- *Integration Testing*: assemble server and test it as a whole

  We will test the server program as a whole, using the provided player program on plank.

  We will run manual tests for each of the test cases that we wish to test, and save the log file outputted by the server in each test case to store the testing results.

  Test Cases we plan to test:

    * Invalid number of command line arguments
    * Invalid command line arguments (invalid file)
    * Adding more than max number of players
    * Running the game with small maps to check gold allocation
    * Players quitting and removing their symbol from the board
    * Spectators quitting
    * Spectators being replaced
    * Players moving on top of one another (swap)
    * Entering and exiting passage ways
    * Visibility of gold piles changes if player leaves them behind
    * Ending the game when all gold is picked up
    * Other possible sources of error
    * Valgrind to check memory leaks

  Specific test procedures will be described in a separate document titled TESTING.md. Please refer to this document for more information on how the Nuggets Project will be tested.
