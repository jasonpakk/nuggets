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
*Input:* the inputs are command-line parameters and the contents of the server specified by the first command-line parameter. See the User Interface above. The server then accepts messages from the clients.

*Output:* Server will show a grid with points consisting of boundaries and passages on to an ASCII screen. The server sends output messages to the clients on any updates to the grid or overall gameplay.

##### Client
*Input:* Keyboard presses that correspond to movements of the player.

*Output:* Client will show the room in the grid that the player is in, with points consisting of boundaries and passages; on to an ASCII screen that updates every time the client sends a message. The room’s visibility depends on the position of the player, and parts of the room may be hidden at the start.

### Functional Decomposition into Modules

We anticipate the following modules or functions:

 1. *main*, which parses arguments and initializes other modules
 2. *maploader*, which loads the map file
 3. *mapinitializer*, which drops the gold piles on random spots of the rooms depending on the seed
 4. *networkinitializer*, which initializes the network and announces the port
 5. *refresh*, which updates the game state and sends it back to the clients
 6. *accept*, which lets players or spectators into the game

And some helper modules that provide data structures:

  1. ***counter*** of players and their scores
  2. ***hashtable*** of players' names with their counter id
  3. ***goldpile*** to store data about each gold pile
  4. ***message*** to handle client-server communication
  5. ***grid*** to represent the map

  ### Pseudo Code for logic/algorithmic flow

  The server will run as follows:

  1. Start from a command line as shown in the User Interface
  2. Parse the command line, validate parameters, initialize other modules
  3. Load the map at `map.txt` and if it fails , take an appropriate action
  4. Initialize the map by dropping the gold piles at random spots using the seed
  5. Initialize the server and announce the port, storing the port in a server file
  6. Refresh the server state, waiting for new players or messages,
  	6.1. if the maxPlayers or maxSpectators is full, do nothing or tell current spectator to leave respectively. Otherwise, accept the players into the server.
  	6.2. If a new player has successfully joined, truncate their name and assign them a letter
        	6.2.1. Send OK message to client with their assigned letter
      	6.3. Send GRID, GOLD, and DISPLAY messages to client
      	8.4. listen to the player and spectator messages and respond
          	6.4.1. When the client sends a quit message, let the client out and remove their symbol.
        	6.4.2. When the player sends a move message
              		6.4.2.1. Check if the move is possible and
                  		6.4.2.1.1. If the character isn’t capitalized, move the player by 1 in the direction of the character.
                  		6.4.2.1.2. If the character is capitalized, move the player as far as possible in the direction of the character.
              		6.4.2.2 Check for gold that was picked up as a result of the move
              		6.4.2.3 Update grid to new positions of players and new amount of gold
                  		6.4.2.3.1 Send GOLD and DISPLAY messages to all clients
          	6.4.3 When there are no more gold piles
              		6.4.3.1 Quit the game for each client.
              		6.4.3.2 Send the clients the summary of the game

  7. Close Server
  

### Dataflow through modules

 1. *main* parses parameters and passes them to the maploader, and then loops over refresh after networkinitializer is done.
 2. *maploader* assumes the map is valid, then stores the NR and NC of the map, and after getting the values of NR and NC, passes the values to mapinitializer.
 3. *mapinitializer* randomly swaps `.` values with gold piles on the map, and calls networkinitializer.
 4. *networkinitializer* hosts the server and announces the port.
 5. *refresh* updates the game status and reads messages sent to the server. It handles the messages appropriately using helper functions:
5a. Tries to accept player or spectator trying to join
5b. Keystroke input from player (movement or quit)

### Major Data Structures

***counter*** of players and their scores  
***hashtable*** of players' names with their counter id  
***message*** to handle client-server communication  
***grid*** to represent the map  
NR X NC matrix array representing each coordinate  
***goldpile[]*** array of gold piles  
***goldpile***   
X coordinate  
Y coordinate  
Amount of gold

### Testing Plan
- *Unit Testing*: small test program to test each module to make sure it does what it’s supposed to do. Additionally, a test program included will ensuring grid functionality and test individual modules and functions used for server.

- *Integration Testing*: assemble server and test it as a whole. Ensure that the visibility and movement is functioning correctly. Print "progress" indicators as the game proceeds (e.g. print when gold is collected, when a player connects/disconnects).
