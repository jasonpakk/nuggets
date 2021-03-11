# Nuggets: Server Implementation Spec

### Contents

This Implementation Spec contains the following:
  1. General Overview
  2. Pseudocode for Major Components
      * ***main***
      * ***play_game***
      * ***handleMessage***
      * ***parse_message***
      * ***generate_position***
      * ***add_player***
      * ***generate_gold***
      * ***move***
      * ***pickup_gold***
      * ***send_grid***
      * ***send_gold***
      * ***send_display***
      * ***refresh***
      * ***send_game_result***
  3. Function Prototypes and their Parameters
  4. Data Structures
      * Parameters
      * Pseudocode for Major Components
  5. Resource Management
  6. Security and Privacy Properties
  7. Error Handling and Recovery
  8. Persistent Storage

### Overview

**SERVER:**

The server for the Nuggets game initializes a server using the ***message*** module to handle client to server communication for the Nuggets game. The server initializes the network and announces the port number for clients to join. The server builds the game map by reading from a text file and handles sending the game map to the clients who will handle the display. The server reacts to inbound messages from clients appropriately and updates all clients with any changes to the game map or amount of gold available.


### Pseudo code for major components

### ***main***
1. Check number of command-line arguments to ensure user used proper usage syntax:

    * `./server map.txt [seed]`

2. Verify arguments; if error, exit non-zero

3. Initialize the ***game_t*** structure using the map filename

4. Initialize the game map by reading the text file and building a ***grid_struct_t***

5. If the user provided a seed, use if for the random number generator

6. If a seed was not provided, generate our own seed by calling ***getpid()***

7. Generate gold piles in the map by calling *generate_gold*

8. Start gameplay by calling *play_game*

9. At conclusion of the game, free memory all the memory used

### ***play_game***
1. Initialize the ***log*** module to log messages to

2. Initialize the network using the ***message*** module

3. Announce the port number

4. Wait for incoming messages from clients and react to each inbound message appropriately

5. Close the network once the game has concluded

### ***handleMessage***
1. Read the incoming message

2. Call *parse_message* to handle the specific message

3. If the game play has ended, return true to end the ***message*** module's looping for messages

### ***parse_message***
1. Parse the message string to obtain the command (PLAY, KEY, or SPECTATE)

2. Parse the message string to obtain what came after the command

3. If the command was PLAY:

    3.1. Ensure we haven't reached max number of players, otherwise send error message

    3.2. Ensure a player name was provided, otherwise send error message

    3.3. Truncate player name to MaxNameLength

    3.4. Replace nongraph/nonblank characters to an underscore

    3.5. Call *add_player* to add the player to our game using their parsed name

4. If the command was SPECTATE:

    4.1. Initialize a new spectator

    4.2. If a spectator currently exists, send them a QUIT message and remove them

    4.3. Assign the new spectator as our current spectator

    4.4. Send the spectator a GRID message

    4.5. Call *refresh* to send the spectator a GOLD and DISPLAY message

5. If the command was KEY

    5.1. Determine what key was pressed by reading what follows the KEY message

    5.2. If the key was "Q", remove the player or spectator who sent the quit message

    5.3 If the key was a movement key, call *move* using the direction we want to move towards

    5.4 If the key was an invalid key, send an error message

6. If a message that doesn't match the syntax specified in the specs was received, send an error message

### ***generate_position***
1. Generate a random x position in the current map

2. Generate a random y position in the current map

3. Using the randomly generated (x,y), obtain the character at that location

4. If the character doesn't match the character we are looking for, repeat steps 1-3

5. If the character we are looking for was obtained, return the position at which the character was found

### ***add_player***
1. Call *generate_position* to find an available room spot

2. Instantiate a new ***server_player_t***, initializing their starting position to the position obtained above

3. Add the player to the hashtable of players using its port number

4. Add the player to the set of players using its symbol

5. Send the accept message ("OK L") to the player

6. Add the player's symbol to the game grid

7. Initialize a grid for the player to track visibility

8. Send the spectator a GRID message

9. Call *refresh* to send the spectator a GOLD and DISPLAY message

10. Update the current symbol and player number to use for the next player that is added

### ***generate_gold***
1. Obtain the total number of room spots available in the current map

2. If the number of available rooms spots is less than the minimum number of gold piles we must generate, return error

3. If the number of available room spots is less than the maximum number of gold piles we must generate, set our max number of gold piles to generate as the number of available room spots
4. Otherwise, just use the given maximum number of gold piles we must generate

5. Randomly generate a number of gold piles to created between our values for the min & max number of gold piles to generate

6. Create an array, using the number of gold piles we just generated as the array size

7. Start off by adding one gold to each pile in the array

8. Randomly add gold to the piles in the array until all of the gold we need to allocate has been used

9. For each gold pile in the array, add it to the game grid

### ***move***
1. Obtain the current position of the player we are trying to move

2. Obtain the new position that the player is trying to move towards

3. Obtain the char at this new position

4. If this char is another player (alphabet):

    4.1. Obtain the second player that we are swapping places with

    4.2. Update the current player's variables regarding its location with the variables of the second player

    4.3. Update the second player's variables regarding its location with the variables of the current player

    4.4. Swap the two symbols in the grid map

5. If this char is a passage (#):

    5.1. Determine whether the player is entering a new passage as a result of this movement

    5.2. If so, update the boolean that tracks whether the player is currently in a passage as true

    5.3. Otherwise, just swap the player symbol with the char at the new location in the grid map

6. If this char is a room spot (.):

    6.1. Determine whether the player is exiting a passage as a result of this movement

    6.2. If so, update the boolean that tracks whether the player is currently in a passage as false

    6.3. Otherwise, just swap the player symbol with the char at the new location in the grid map

7. If this char is a pile of gold (*):

    7.1 Determine whether the player is exiting a passage as a result of this movement

    7.2 Call *pickup_gold* to appropriately handle picking up a new gold pile

8. Update the player's previous position with the current position

9. Update the player's current position with the new position the player has moved into

10. Call *refresh* to send everyone a new GOLD and DISPLAY message

### ***pickup_gold***
1. If necessary, replace the gold char in the game map with a room symbol

2. Obtain the amount of gold that was contained in the gold pile

3. Update the amount of gold in this pile to zero

4. Subtract the amount of gold in this pile from the total gold remaining

5. Update the player's gold count using the amount of gold they just picked up

### ***send_grid***
1. Obtain the size of the grid (nrows and ncols)

2. Send the size of the grid to the client using the ***message*** module

### ***send_gold***
1. Obtain the amount of gold collected, amount of gold in purse, and the amount of gold left for a player

2. Send this data about gold to the client using the ***message*** module

### ***send_display***
1. Obtain the map based on a player's visibility as a string

2. Send the map state based on the client's visibility using the ***message*** module

### ***refresh***
1. For each active player, call *send_display* to send the player a display of the grid

2. If there is a spectator, call *send_display* to send the spectator a display of the grid

3. If no more gold remains, call *send_game_result* to send the game result to all players and end the game

### ***send_game_result***
1. Send each player the result string that specifies how much gold each player collected

2. If there is a spectator, send the spectator the result string as well

### Function Prototypes and their Parameters

The code is broken down into the following functions:
```c
// function prototypes
int play_game();
static bool handleMessage(void *arg, const addr_t from, const char *message);
static void parse_message(const char *message, addr_t *address);

static position_t* generate_position(grid_struct_t *grid_struct, char valid_symbol);
static int generate_gold(grid_struct_t *grid_struct);
static void add_player(addr_t *address, char* player_name);

static bool move(addr_t *address, int x, int y);
static server_player_t *get_player(addr_t *address);
static void pickup_gold(server_player_t *curr, position_t *pos, bool overwrite);

static void send_grid(addr_t address);
static void send_gold(server_player_t* player);
static void send_display(server_player_t* player);
static void refresh();
static void refresh_helper(void *arg, const char *key, void *item);

static char* game_result_string();
static void game_result_string_helper(void *arg, const char *key, void *item);
static void send_game_result(char* result_string);
static void send_game_result_helper(void *arg, const char *key, void *item);

game_t* game_new(char *map_filename);
void game_delete(game_t *game);
static void game_delete_helper(void *item);
```

### Data Structures

### game
***game_t*** is a data structure we created to store data regarding the Nuggets gameplay. It was the global variable we used for our implementation.

```c
typedef struct game {
  bool playing_game;  // whether the game is still in session
  char* map_filename; // name of file to read map from
  int gold_remaining;   // amount of gold left
  int gold_pile_number;  // amount of gold piles
  int player_number;    // current number of players
  char curr_symbol;     // current symbol to assign a player
  grid_struct_t *main_grid;   // game grid that sees all
  hashtable_t *players;   // stores all players; key is their address
  set_t* symbol_to_player;  // stores all players; key is their symbol
  server_player_t *spectator;  // pointer to the spectator watching the game
} game_t;
```

### hashtable
***hashtable_t*** is a data structure provided by previous labs in CS50.

```c
typedef struct hashtable {
  int num_slots;          // number of slots in the table
  set_t **table;          // table[num_slots] of set_t*
} hashtable_t;
```

### set
***set_t*** is a data structure provided by previous labs in CS50.

```c
typedef struct set {
  struct setnode *head;   // head of the set
} set_t;
```

### grid_struct
***grid_struct_t*** is a module we created to store the game map for the Nuggets gameplay. It stores the game map as a 2D Array of ***point_t*** pointers. The ***grid_struct_t*** is also used to track the visibility of each grid point in the game map for each player. As a result, the pseudocode for these major components of the ***grid_struct_t*** module are provided below.

```c
typedef struct grid_struct {
  int nR; // number of rows
  int nC; // number of columns
  int room_spot; // number of room spots
  point_t*** grid; // 2d array of pointers to points
} grid_struct_t;
```
**Psuedocode for Major Components**

##### ***grid_swap***
1. Check parameters before proceeding, return on error

2. Obtain the symbol at the new position we are trying to swap at

3. If it is a symbol that can be swapped (not a wall), update the symbol at the current position with the new symbol

4. Update the symbol at the new position with the old symbol

##### ***grid_visibility***
1. Check parameters before proceeding, return on error

2. Create a line segment between the point we are currently located at to a point in the grid we want to determine the visibility of

3. For each column (x) value in the line segment:

    3.1. Obtain its respective y value to get the (x,y) coordinate

    3.2. If this (x,y) coordinate intersects a grid point exactly, check if this grid point is a room spot

    3.3. If this (x,y) coordinate passes between a pair of grid points, check both grid points on whether it is a room spot

    3.4. If not a room spot(s), return false since our vision is blocked

4. For each row (y) value in the line segment:

    4.1 Obtain its respective x value to get the (x,y) coordinate

    4.2. If this (x,y) coordinate intersects a grid point exactly, check if this grid point is a room spot

    4.3. If this (x,y) coordinate passes between a pair of grid points, check both grid points on whether it is a room spot

    4.4. If not a room spot(s), return false since our vision is blocked

5. Only if it passes all these tests, we can set the visibility of the point in the grid to true

6. Repeat steps 2-4 with every other point in the grid

##### ***grid_string_player***
1. Allocate memory for the map string to send to a player

2. For each point in the grid:

    2.1. If this point is the current location of the player, print an '@' as defined in the specs

    2.2. Otherwise, check the visibility of the point from the current location of the player

    2.3. If the player has never seen this point before, print an empty space

    2.4. If the player has seen this point before, do one of two things:  

      * 2.4.1. If the point is a pile of gold, only print the gold if the player can see it from its current location. Otherwise, print a normal room space.

      * 2.4.2. For all other points, as long as the player has seen this point before, print its respective character

### position
***position_t*** is a data structure provided with the ***grid_struct_t*** module that keeps track of the x and y coordinates. This data structure contains basic setter and getter methods to update and access the coordinates.

```c
typedef struct position {
  int x;   // x coord
  int y;   // y coord
} position_t;
```

### point
***point_t*** is a data structure used by the ***grid_struct_t*** module in its 2D Array. A single ***point_t*** structure represents a single point on the game grid.

```c
typedef struct point {
  char c;     // char at the point
  bool seen_before;   // has the player ever seen this point before
  bool visible_now;   // is the point visible from a player's current position
  int gold_number;  // amount of gold (gold piles)
} point_t;
```

### server_player
***server_player_t*** is a data structure we created to store the data of each player playing the Nuggets Game.  map for the Nuggets gameplay. This data structure contains basic setter and getter methods to update and access the data of the server_player.

```c
typedef struct server_player {
  char *name;   // player name
  char symbol;  // player symbol on the game map
  addr_t address; // address to send/recieves messages from
  int gold_number;  // total gold in player purse
  int gold_picked_up; // amt of gold just picked up by player
  bool active;    // whether the player is currently playing
  position_t *pos;    // current player position
  grid_struct_t *grid;    // player grid that tracks visibility
  bool in_passage;    // whether the player is currently in a passage way
} server_player_t;
```

### Resource Management

The server handles client-server communication using the ***message*** module which provides a message-passing abstraction among Internet hosts.

Messages are sent via UDP and thus the network protocol for the Nuggets Game runs over UDP/IP. That is, the user datagram protocol over the Internet Protocol.

In either protocol, communication occurs between two endpoints; the address of an endpoint is a pair host IP address, port number. UDP carries datagrams from one port on one host to another port on another host (well, they could be the same host). A datagram can hold zero to 65,507 bytes.

The server also uses the ***log*** module in order to log useful information that can be saved in a logfile. The user specifies which file to print the log output to.

An example approach to using the logfile would be:
```bash
      ./server 2>server.log map.txt
```

### Security and Privacy Properties

The only potential private information in the system is the IP address of the requesting client. The user specifies which file to print the log output to, so the user can purge the logfile if they wish to.


### Error Handling

To prevent error, the server verifies arguments provided in the command line before proceeding with gameplay. If any error, the program provides a useful error message and exits non-zero.

The same is true with the public functions of the program. If any argument is invalid or there is a malloc error, the program recovers gracefully by printing a useful error message and exiting non-zero.

We do not, however, check parameters for static functions, since the only time they are called is within other functions in the program that verify the arguments it sends before calling these static methods. We do, however, recover gracefully from any malloc errors in static functions by printing a useful error message and exiting non-zero.

In case the server receives a message that does not follow the specified protocol, the server is robust and recovers gracefully. The program does not crash with any malformatted message. Instead, the program logs an error and ignores the message.

### Persistent Storage

The server does not print anything to stdout other than what is required for game play. The only time the server prints to stdout, therefore, is to announce the port number.

Instead, the server logs useful information that can be saved in a logfile using the ***log*** module. It is possible to output to different log file, based on the specifications provided by the user when the server program is called. Read the description under "Resource Management" for more information on how to use the logfile.
