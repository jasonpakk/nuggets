# Nuggets: Server Implementation Spec

### Contents

This Implementation Spec contains the following:
  1. General Overview
  2. Pseudocode for Major Components
      * ***main***
      * ***play_game***
      * ***handleMessage***
      * ***parse_message***
      * ***send_grid***
      * ***send_gold***
      * ***send_display***
  3. Function Prototypes and their Parameters
  4. Data Structures
  5. Resource Management
  6. Security and Privacy Properties
  7. Error Handling and Recovery
  8. Persistent Storage

### Overview

**SERVER**
The server for the Nuggets game initializes a server using the ***message*** module to handle client to server communication for the Nuggets game. The server initializes the network and announces the port number for clients to join. The server builds the game map by reading from a text file and handles sending the game map to the clients who will handle the display. The server reacts to inbound messages from clients appropriately and updates all clients with any changes to the game map or amount of gold available.


### Pseudo code for major components

### ***main***
1. Check number of command-line arguments to ensure user used proper usage syntax:
    * `./server map.txt [seed]`
2. Verify arguments; if error, exit non-zero
3. Initialize the game map by reading the text file and building a ***grid_struct_t***
4. Start gameplay by calling *play_game()*

### ***play_game***
1. If a seed was provided, seed the RNG with provided seed
2. Initialize the network using the ***message*** module and announce the port number
3. Wait for incoming messages from clients and react to each inbound message appropriately
4. Close the network once the game has concluded

### ***handleMessage***
1. Read the incoming message
2. Call *parse_message* to handle the specific message

### ***parse_message***
1. Determine what command the message is calling
2. React appropriately by calling the proper helper method to handle the specific message
3. If the message is one that does not follow gameplay syntax, send an error message to the client

### ***send_grid***
1. Obtain the size of the grid (nrows and ncols)
2. Send the size of the grid to the client using the ***message*** module

### ***send_gold***
1. Obtain the amount of gold collected, amount of gold in purse, and the amount of gold left
2. Send the data about gold to the client using the ***message*** module

### ***send_display***
1. Obtain the current state of the map
2. Send the map state based on the client's visibility using the ***message*** module

### Function Prototypes and their Parameters

The code is broken down into the following functions:
```c
// function prototype declarations
int play_game(int seed);
static bool handleMessage(void *arg, const addr_t from, const char *message);
int parse_message(const char *message, addr_t *address);
void send_grid(addr_t *address);
void send_gold(addr_t *address, int n, int p, int r);
void send_display(addr_t *address);
```

### Data Structures

### hashtable
```c
typedef struct hashtable {
  int num_slots;          // number of slots in the table
  set_t **table;          // table[num_slots] of set_t*
} hashtable_t;
```

### set
```c
typedef struct set {
  struct setnode *head;   // head of the set
} set_t;
```

### set
```c
typedef struct grid_struct {
  int nR; // number of rows
  int nC; // number of columns
  char** grid; // array of pointers to char pointers
} grid_struct_t;
```

### position
```c
typedef struct position {
  int x;
  int y;
} position_t;
```

### player
```c
typedef struct player {
  char *name;
  char symbol;    // representation on game board
  addr_t *address;
  int gold_number;   // amount of gold
  bool active;      // whether a spectator
  position_t *pos;  // current position on game board
} player_t;
```

### game
```c
typedef struct game {
  int gold_remaining;
  int player_number;  // num of players
  char curr_symbol;
  grid_struct_t *grid;  // game board
  hashtable_t *players; // stores players
  player_t *spectator;
} game_t;
```

### Resource Management

The server handles client-server communication using the ***message*** module which provides a message-passing abstraction among Internet hosts.

Messages are sent via UDP and thus the network protocol for the Nuggets Game runs over UDP/IP. That is, the user datagram protocol over the Internet Protocol.

In either protocol, communication occurs between two endpoints; the address of an endpoint is a pair host IP address, port number. UDP carries datagrams from one port on one host to another port on another host (well, they could be the same host). A datagram can hold zero to 65,507 bytes.

The server also uses the ***log*** module in order to log useful information that can be saved in a logfile. The user specifies which file to print the log output to.


### Security and Privacy Properties

The only potential private information in the system is the IP address of the requesting client. The user specifies which file to print the log output to, so the user can purge the logfile if they wish to.


### Error Handling

To prevent error, the server verifies arguments before proceeding with gameplay. If any error, the program provides a useful error message and exits non-zero.

The same is true with the functions of the program. If any argument is invalid or there is a malloc error, the program recovers gracefully by printing a useful error message and exiting non-zero.

In case of any message not following the specified protocol, the server is robust. The program does not crash with any malformatted message. Instead, the program logs an error and ignores the message.

### Persistent Storage

The server does not print anything to stdout other than what is required for game play. Instead, the server logs useful information that can be saved in a logfile using the ***log*** module. It is possible to output to different log file, based on the specifications provided by the user when the server program is called.
