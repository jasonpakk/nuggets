/*
 * server.c      Team JEN      March 2021
 *
 * server.c  - CS 50 "SERVER" module for NUGGETS game
 * usage: ./server map.txt [seed]
 * Read the README.md and IMPLEMENTATION.md for more information.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include "memory.h"
#include "message.h"
#include "log.h"
#include "hashtable.h"
#include "set.h"
#include "grid.h"
#include "server_player.h"

/* struct that stores data about the overall game
*/
typedef struct game {
  bool playing_game;  // whether the game is still in session
  char* map_filename; // name of file to read map from
  int gold_remaining;   // amount of gold left
  int gold_pile_number;  // amount of gold piles
  int player_number;    // total number of players
  int n_active_players;   // current number of active players
  char curr_symbol;     // current symbol to assign a player
  grid_struct_t *main_grid;   // game grid that sees all
  hashtable_t *players;   // stores all players; key is their address
  set_t* symbol_to_player;  // stores all players; key is their symbol
  server_player_t *spectator;  // pointer to the spectator watching the game
} game_t;

// global variable
static game_t* game;

// local constants
static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 26;      // maximum number of players
static const int GoldTotal = 250;      // amount of gold in the game
static const int GoldMinNumPiles = 10; // minimum number of gold piles
static const int GoldMaxNumPiles = 30; // maximum number of gold piles

// function prototypes
int play_game();
static bool handleMessage(void *arg, const addr_t from, const char *message);
static void parse_message(const char *message, addr_t *address);

static position_t* generate_position(grid_struct_t *grid_struct, char valid_symbol);
static int generate_gold(grid_struct_t *grid_struct);
static int add_player(addr_t *address, char* player_name);

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

/*
 * main function of program; checks whether arguments
 * are appropriate values, and initalizes modules used
 * for playing the nuggets game; frees used memory at the end
 */
int
main(const int argc, const char *argv[])
{
  if (argc == 2 || argc == 3) {
    // store name of map file
    char* map_file = (char*)malloc(1+ strlen(argv[1])*sizeof(char));
    strcpy(map_file, argv[1]);

    // initialize game
    game = game_new(map_file);

    // initialize first grid
    grid_struct_t *main_grid = grid_struct_new(map_file);
    if (main_grid == NULL) {
      return 1;
    }

      // all points are seen
      bool loaded = grid_load(main_grid, map_file, true);
      if (!loaded) {
        fprintf(stderr, "error loading grid.\n");
        return 1;
      }
      game->main_grid = main_grid;

    // no seed
    if (argc == 2) {
      // generate a seed
      srand(getpid());
    } else {
      //seed provided, scan it in and check it is positive
      int seed = atoi(argv[2]);
      if (seed <= 0) {
        fprintf(stderr, "the seed must be a positive integer.\n");
        return 1;
      }
      srand(seed);
    }

    // generate gold piles
    game->gold_pile_number = generate_gold(game->main_grid);
    if (game->gold_pile_number == 1) {
      fprintf(stderr, "grid must have at least 10 available room spots to place gold\n");
      return 2;
    }

    if(play_game() != 0) { // run the game
      fprintf(stderr, "error while playing the nuggets game\n");
    }
    game_delete(game); // free memory at end

  } else {
    // wrong number of arguments
    fprintf(stderr, "usage: ./server map.txt [seed]\n");
    return 2;
  }
  return 0;
}

/* ***************** play_game ********************** */
/* Initalizes the server using the message module and
 * waits for messages from the client to come in. We shut down
 * the message module on any error or if game play has ended.
 *
 * We RETURN: 0 on successful game play; otherwise we return
 * a non-zero integer.
 */
int
play_game()
{
  log_init(stderr);
  // initialize the message module
  int ourPort = message_init(stderr);
  if (ourPort == 0) {
    log_v("failure to initalize message module.");
    return 1;
  }
  // announce the port number
  printf("waiting on port %d for contact....\n", ourPort);
  // address of the other side of this communication
  addr_t other = message_noAddr(); // no correspondent yet

  // Loop, waiting for messages; If message recieved, take care of it
  // using handleMessage function. We use the 'arg' parameter to carry a pointer
  // to 'other', which allows handleMessage to use it appropriately.
  bool ok = message_loop(&other, 0, NULL, NULL, handleMessage);

  // shut down the modules
  message_done();
  log_done();

  return ok? 0 : 1; // status code depends on result of message_loop
}

/**************** handleMessage ****************/
/* Datagram received; parse the message.
 * We use 'arg' to carry an addr_t referring to the 'other' correspondent.
 * If 'other' is a valid client, leave it unchanged.
 * If 'other' is not a valid client, update *from to this sender.
 * Return true if the message loop should exit, otherwise false.
 * e.g., return true if fatal error, or if game play has concluded.
 */
static bool
handleMessage(void *arg, const addr_t from, const char *message)
{
  addr_t *otherp = (addr_t *)arg;
  if (otherp == NULL) { // defensive
    log_v("handleMessage called with arg=NULL");
    return true;
  }
  // this sender becomes our correspondent, henceforth
  *otherp = from;
  parse_message(message, otherp);

  // if game has ended, return true to exit the message loop
  if(game->playing_game == false) {
    return true;
  }
  return false;
}

/**************** parse_message ****************/
/* Parse the message recieved and respond appropriately
 * by calling the proper helper function.
 *
 * Caller provides:
 *   valid char pointer representing the message string
`*   valid address pointer of the player who sent the message
 * Notes:
 *   The message should be sent according to the syntax defined
 *   in the specs. On any message that isn't appropriate, we
 *   send an error message back to the client.
 */
static void
parse_message(const char *message, addr_t *address)
{
  // Player to Server Commands
  char *play = "PLAY";
  char *key = "KEY";
  char *spectate = "SPECTATE";

  // find the command by looping through message string
  // until a space char is reached
  char *command = (char*) message;
  char *remainder = (char*) message;
  while (!isspace(*remainder)) {
    remainder++;
  }
  // found end of word, put end of string character
  *remainder = '\0';
  //increment remainder to find start of the rest of message
  remainder++;

  // 1) Command to ADD A NEW PLAYER
  if (strcmp(command, play) == 0) {
    // write error message, too many players
    if (game->player_number >= MaxPlayers) {
      // write error message, too many players
      message_send(*address, "QUIT Game is full: no more players can join.");

    // write error message, name not provided
    } else if(strlen(remainder) == 0) {
      message_send(*address, "QUIT Sorry - you must provide player's name.");
    }

    // otherwise, we can add this player to the game
    else {
      // truncate player name to MaxNameLength
      if(strlen(remainder) > MaxNameLength) {
        remainder[MaxNameLength] = '\0';
      }
      // update nongraph/nonblank chars to underscores
      for(int i = 0; i < strlen(remainder); i++) {
        char c = remainder[i];
        if(!isgraph(c) && !isblank(c)) {
          remainder[i] = '_';
        }
      }
      // add player to game using their parsed player name
      if (add_player(address, remainder) != 0) {
        message_send(*address, "QUIT Game is full: no more space on the grid for players to join.");
      }
    }

  // 2) Command to ADD A NEW SPECTATOR
  } else if (strcmp(command, spectate) == 0) {
    // initalize new spectator
    addr_t new = *address;
    server_player_t *new_spectator = server_player_new(new, spectate, '!', false, NULL);
    server_player_setGrid(new_spectator, game->main_grid);

    // if there is already a spectator, replace them
   if (game->spectator != NULL) {
     message_send(server_player_getAddress(game->spectator), "QUIT You have been replaced by a new spectator.");
     server_spectator_delete(game->spectator);
    }
    game->spectator = new_spectator;

    // send spectator a map to draw
    send_grid(*address);
    refresh();

  // 3) Command to HANDLE A KEY PRESS
  } else if (strcmp(command, key) == 0) {
    // read what the key is
    char c = *remainder;

    // QUIT key command
    if (c == 'Q') {

      // send appropriate message depending on if client is spectator or player
      if(game->spectator != NULL && message_eqAddr(server_player_getAddress(game->spectator), *address)) {
        message_send(*address, "QUIT Thanks for watching!");
        server_spectator_delete(game->spectator);
        game->spectator = NULL;
      } else {
        message_send(*address, "QUIT Thanks for playing!");

        // get pointer to player who just quit using their address
        char portnum1[100];
        sprintf(portnum1, "%d",ntohs((*address).sin_port));
        server_player_t *curr = hashtable_find(game->players, portnum1);
        server_player_setActive(curr, false); // no longer active
        game->n_active_players--;

        // remove their symbol from the main grid
        position_t *playerPos = server_player_getPos(curr);
        char toReplace = grid_get_point_c(server_player_getGrid(curr), pos_get_x(playerPos), pos_get_y(playerPos));
        grid_set_character(game->main_grid, toReplace, playerPos);
        refresh();
      }
    }

    // MOVEMENT key command
    else if (game->spectator == NULL || !message_eqAddr(server_player_getAddress(game->spectator), *address)) {

      // switch statement to handle different keys
      switch (c) {
        // Move to the left by 1
        case 'h':
          move(address, -1, 0);
          break;
        // Move to the left (until not possible)
        case 'H':
          while(move(address, -1, 0));
          break;

        // Move right
        case 'l':
          move(address, 1, 0);
          break;
        // Move right (until not possible)
        case 'L':
          while(move(address, 1, 0));
          break;

        // Move up
        case 'k':
          move(address, 0, -1);
          break;
        // Move up (until not possible)
        case 'K':
          while(move(address, 0, -1));
          break;

        // Move down
        case 'j':
          move(address, 0, 1);
          break;
        // Move down (until not possible)
        case 'J':
          while(move(address, 0, 1));
          break;

        // move diagonally up and left
        case 'y':
          move(address, -1, -1);
          break;
        // move diagonally up and left (until not possible)
        case 'Y':
          while(move(address, -1, -1));
          break;

        // move diagonally up and right
        case 'u':
          move(address, 1, -1);
          break;
        // move diagonally up and right (until not possible)
        case 'U':
          while(move(address, 1, -1));
          break;

        // move diagonally down and left
        case 'b':
          move(address, -1, 1);
          break;
        // move diagonally down and left (until not possible)
        case 'B':
          while(move(address, -1, 1));
          break;

        // move diagonally down and right
        case 'n':
          move(address, 1, 1);
          break;
        // move diagonally down and right (until not possible)
        case 'N':
          while(move(address, 1, 1));
          break;

        // bad key, send error message
        default:
          message_send(*address, "ERROR invalid keystroke");
          break;
      }
    }

  // recieved a message that does not match our game syntax, send error
  } else {
    message_send(*address, "ERROR unable to understand message");
  }
}

/**************** generate_position ****************/
/* Randomly finds a position in the grid that corresponds
 * to the provided symbol.
 *
 * Caller provides:
 *   valid char representing the symbol to search for
`*   valid grid_struct_t pointer representing the grid we
        are searching in
 * We RETURN:
 *    a pointer to the position in the grid where the
      symbol is located
 * Notes:
 *   We assume the user provides a VALID SYMBOL - a symbol
 *   that exists in the grid_struct.
 */
static position_t*
generate_position(grid_struct_t *grid_struct, char valid_symbol)
{
  bool found = false;
  int rand_x;
  int rand_y;
  char possible_point;

  while (found == false) {
    // Create x and y inside the grid row and columns
    rand_x = rand() % grid_get_nC(grid_struct);
    rand_y = rand() % grid_get_nR(grid_struct);

    // Get the point from grid
    possible_point = grid_get_point_c(grid_struct, rand_x, rand_y);

    // Compare the characters
    if (possible_point == valid_symbol) {
      found = true;
      return position_new(rand_x, rand_y);
    }
  }
  return NULL;
}

/**************** generate_gold ****************/
/* Randomly allocates "GoldTotal" into piles of gold in the grid, where
 * the number of piles is between "GoldMinNumPiles" and "GoldMaxNumPiles"
 *
 * Caller provides:
`*   valid grid_struct_t pointer representing the grid we
        are searching in
 * We RETURN:
 *    the number of gold piles generated. We return 1 on any error.
 */
static int
generate_gold(grid_struct_t *grid_struct)
{
  // Obtain number of available room spots where we can place gold
  int room_spot_number = grid_get_room_spot(grid_struct);
  // if map doesn't have enough space for GoldMinNumPiles, exit
  if (room_spot_number < 10) {
    return 1;
  }

  // obtain a max number based on availabile room spots
  int localMaxNumPiles;
  if (room_spot_number < GoldMaxNumPiles) {
    localMaxNumPiles = room_spot_number;
  } else {
    localMaxNumPiles = GoldMaxNumPiles;
  }
  // use rand() to generate random number of gold piles to be created
  int n_gold_piles = (rand() % (localMaxNumPiles - GoldMinNumPiles + 1)) + GoldMinNumPiles;

  // we have n_gold_piles to make, so create an array of that size;
  int gold_pile_array[n_gold_piles - 1];
  for (int i = 0; i < n_gold_piles; i++) {
    gold_pile_array[i] = 1;   // add 1 to every pile
  }

  // generates random number between 0 and number of gold piles - 1
  // to determine how much gold to add for each gold pile
  int pile_num;
  for (int j = 0; j < GoldTotal - n_gold_piles; j++) {
    pile_num = (rand() % (n_gold_piles));
    gold_pile_array[pile_num] = gold_pile_array[pile_num] + 1;
  }

  // add each gold pile to the grid
  for (int k = 0; k <= n_gold_piles - 1; k++) {
    // Get a random position for the pile
    position_t *curr_pile_pos = generate_position(game->main_grid, '.');
    // Change the point for the pile in the grid
    grid_set_character(game->main_grid, '*', curr_pile_pos);
    // Change the gold amount for the pile in the grid
    grid_set_gold(game->main_grid, gold_pile_array[k], curr_pile_pos);
    position_delete(curr_pile_pos);
  }

  return n_gold_piles;
}

/**************** add_player ****************/
/* Adds a new player to a random spot on the grid.
 *
 * Caller provides:
`*   valid addr_t pointer representing the address of the
        player we are adding
     valid char* representing the player name
 */
static int
add_player(addr_t *address, char* player_name)
{
  if (grid_get_room_spot(game->main_grid) - game->n_active_players <= 0) {
    return 1;
  }
  // Find a position to put the player in and create a new player
  position_t *pos;
  if (grid_get_room_spot(game->main_grid) <= game->gold_pile_number) {
    pos = generate_position(game->main_grid, '*');
  } else {
    pos = generate_position(game->main_grid, '.');
  }
  server_player_t *new_player = server_player_new(*address, player_name, game->curr_symbol, true, pos);

  // add player to hashtable (portnum->player)
  char portnum[100];
  sprintf(portnum, "%d",ntohs((*address).sin_port));
  hashtable_insert(game->players, portnum, new_player);
  game->n_active_players++;

  // add player to set (symbol->player)
  char player_symbol[3];
  sprintf(player_symbol, "%c", server_player_getSymbol(new_player));
  set_insert(game->symbol_to_player, player_symbol, new_player);

  // send player the accept message
  char player_info[5];
  sprintf(player_info, "OK %c", server_player_getSymbol(new_player));
  message_send(*address, player_info);

  // add the player symbol to main grid
  grid_set_character(game->main_grid, server_player_getSymbol(new_player), pos);

  // if the player was added into a '*' position, automatically pick up the gold pile.
  pickup_gold(new_player, pos, false);

  // initialize grid for player
  grid_struct_t *player_grid = grid_struct_new(game->map_filename);
  grid_load(player_grid, game->map_filename, false);
  server_player_setGrid(new_player, player_grid);

  // send the player the grid's information
  send_grid(*address);
  refresh();

  // update current letter and player number
  game->curr_symbol += 1;
  game->player_number += 1;

  return 0;
}

/**************** pickup_gold ****************/
/* Pick up the gold pile the player has moved into
 * and update the grid accordingly.
 *
 * Caller provides:
`*   valid player_t pointer, representing the player that picked
 *        up the gold
 *   valid position_t pointer, representing the position where the
 *        gold was picked up
 *   a boolean determining whether we need to replace the gold pile
 *        with a room character '.'
 */
static void
pickup_gold(server_player_t *curr, position_t *pos, bool overwrite)
{
  // if we need to update the gold char with a room symbol
  if (overwrite) {
    grid_set_character(game->main_grid, '.', server_player_getPos(curr));
  }
  // obtain amount of gold picked up
  int gold_picked_up = grid_get_point_gold(game->main_grid, pos_get_x(pos), pos_get_y(pos));
  // update game grid
  grid_set_gold(game->main_grid, 0, pos);
  game->gold_remaining -= gold_picked_up;
  // update player's gold count
  server_player_setGoldNumber(curr, server_player_getGoldNumber(curr) + gold_picked_up);
  server_player_setGoldPickedUp(curr, gold_picked_up);

  if (gold_picked_up > 0) {
    game->gold_pile_number --;
  }
}

/**************** move ****************/
/* Handle player movement across the board.
 *
 * Caller provides:
`*   valid address_t pointer, representing the player's address
 *   valid int value representing how much to increment in the x direction
 *   valid int value representing how much to increment in the y direction
 * We return:
 *    True on successful move, false on any error.
 */
static bool
move(addr_t *address, int x, int y)
{
  // Get the position to the direction of the player
  server_player_t *curr = get_player(address);
  int new_x = pos_get_x(server_player_getPos(curr)) + x;
  int new_y = pos_get_y(server_player_getPos(curr)) + y;

  // Get the character at the next point
  // (grid_get_point_c ensures we aren't out of bounds so we dont check here)
  char c = grid_get_point_c(game->main_grid, new_x, new_y);

  // If the move is valid
  if (c == '.' || isalpha(c) || c == '#' || c == '*') {
    // Swap the player's character with the next point's character
    position_t *new = position_new(new_x, new_y);
    grid_swap(game->main_grid, new, server_player_getPos(curr));

    // If the next point is a player, then update the next player's position
    if (isalpha(c)) {
      // find the player we swapped positions with
      char* player2_symbol = malloc(sizeof(char) + 1);
      sprintf(player2_symbol, "%c", c);
      server_player_t* player2 = set_find(game->symbol_to_player, player2_symbol);
      free(player2_symbol);

      // If the player2 is in a passage, swap the passage booleans and the previous positions of both players
      bool temp_passage = server_player_getInPassage(player2);
      server_player_setInPassage(player2, server_player_getInPassage(curr));
      server_player_setInPassage(curr, temp_passage);

      //reallocate positions for both players
      position_t *curr_pos_p1 = position_new(pos_get_x(server_player_getPos(curr)), pos_get_y(server_player_getPos(curr)));
      position_delete(server_player_getPos(player2));
      position_delete(server_player_getPos(curr));

      // update each player's positions appropriately
      server_player_setPos(player2, curr_pos_p1);
      server_player_setPos(curr, new);

    } else {
      // If the next point is a passage, adjust the grid accordingly
      if (c == '#') {
        // If the player is entering the passage, set the previous position to '.', instead of '#'
        if(!server_player_getInPassage(curr)) {
          grid_set_character(game->main_grid, '.', server_player_getPos(curr));
          server_player_setInPassage(curr, true);
        }
      }

      if (c == '.' ) {
        // If the player is exiting a passage, set the previous position to '#' instead of '.'
        if(server_player_getInPassage(curr)) {
          grid_set_character(game->main_grid, '#', server_player_getPos(curr));
          server_player_setInPassage(curr, false);
        }
      }

      // If the next point is a gold_pile, adjust the grid accordingly
      if (c == '*') {
        pickup_gold(curr, new, true);
        // If the player is exiting a passage, set the previous position to '#' instead of '.'
        if(server_player_getInPassage(curr)) {
          grid_set_character(game->main_grid, '#', server_player_getPos(curr));
          server_player_setInPassage(curr, false);
        }
      }

      // free pointer stored in curr pos before updating
      position_delete(server_player_getPos(curr));
      // Update the player's position variables to reflect the grid.
      server_player_setPos(curr, new);
    }

    refresh();
    return true;
  }
  return false;
}

/**************** get_player ****************/
/* Searches the hashtable of players to find the player
 * that corresponds to the provided address.
 * User provides:
 *      valid address of the player we are searching for
 * We return a pointer to the player found, otherwise NULL.
 */
static server_player_t *get_player(addr_t *address)
{
  // search hashtable for player with the address
  char portnum1[100];
  sprintf(portnum1, "%d",ntohs((*address).sin_port));
  server_player_t *curr = hashtable_find(game->players, portnum1);
  return curr;
}

/**************** send_grid ****************/
/* As defined in the specs, sends a grid message
 * as the following: "GRID nrows ncols"
 *
 * Caller provides:
`*   valid address pointer of the player to send the message
 */
static void
send_grid(addr_t address)
{
  char map_info[256];
  int nC = grid_get_nC(game->main_grid);
  int nR = grid_get_nR(game->main_grid);
  sprintf(map_info, "GRID %d %d", nR, nC);
  message_send(address, map_info);
}

/**************** send_gold ****************/
/* As defined in the specs, send a gold message
 * as the following: "GOLD n p r".
 *    Where n = number of gold just picked up
 *          p = number of gold in purse
 *          r = number of gold remaining
 *
 * Caller provides:
`*   valid player pointer to the player to send the message
 */
static void
send_gold(server_player_t* player)
{
  int n = server_player_getGoldPickedUp(player);
  int p = server_player_getGoldNumber(player);
  int r = game->gold_remaining;
  char gold_info[256];
  sprintf(gold_info, "GOLD %d %d %d", n, p, r);
  message_send(server_player_getAddress(player), gold_info);
  // reset to 0; no longer picked up new gold
  server_player_setGoldPickedUp(player, 0);
}

/**************** send_display ****************/
/* As defined in the specs, send a display message
 * as the following: "DISPLAY\nstring".
 *    Where string is a multi-line textual representation
 *    of the grid as known/seen by this client
 *
 * Caller provides:
`*   valid player pointer to the player to send the message
 */
static void
send_display(server_player_t* player)
{
  send_gold(player); // first inform player of their gold

  char *string;
  // if a spectator, retrieve string of entire grid
  if(player == game->spectator) {
    string = grid_string(game->main_grid);
  // if a regular player, retrieve string based on visibility
  } else {
    grid_struct_t *grid = server_player_getGrid(player);
    grid_visibility(grid, server_player_getPos(player));
    string = grid_string_player(game->main_grid, grid, server_player_getPos(player));
  }

  // create string to send the grid message
  char *display_info = malloc(strlen(string) * sizeof(char*));
  sprintf(display_info, "DISPLAY\n%s", string);
  message_send(server_player_getAddress(player), display_info);
  free(string);
  free(display_info);
}

/**************** refresh ****************/
/* Send the display to all active players and the spectator.
 * If there is no more gold remaining, we send the game result
 * to all players and end the game.
 */
static void
refresh()
{
  // send display to all players
  hashtable_iterate(game->players, NULL, refresh_helper);

  // send updates to spectator
  if(game->spectator != NULL) {
    send_display(game->spectator);
  }

  // if no more gold remains, send game result and end the game
  if (game->gold_remaining == 0) {
    char* result = game_result_string();
    send_game_result(result);
    free(result);
    game->playing_game = false;
  }
}

/**************** refresh_helper ****************/
/* Helper method for refresh().
 * Used to iterate through players in the hashtable. If
 * the player is active, send them the display of the grid.
 */
static void
refresh_helper(void *arg, const char *key, void *item)
{
  if (key != NULL) {
    server_player_t* curr = item;
    // if player is active, send the player a display of the grid
    // since send_display also sends gold messages, active players
    // also recieve updates on the amount of gold they currently have
    if(server_player_getActive(curr) == true) {
      send_display(curr);
    }
  }
}

/**************** game_result_string ****************/
/* Format a game result string that details how much gold
 * each player has collected.
 *
 * We return:
`*   A valid char pointer of the result message
 */
static char*
game_result_string()
{
  char* result_string = malloc((14 + MaxNameLength * sizeof(char))*(game->player_number+1));
  char* first_line = "QUIT GAME OVER:\n";
  strcpy(result_string, first_line);
  hashtable_iterate(game->players, result_string, game_result_string_helper);
  return result_string;
}

/**************** game_result_string_helper ****************/
/* Helper method for game_result_string().
 * Used for iterating through the hashtable of players
 * to obtain how much gold each player has collected and add
 * that information to the result string.
 */
static void
game_result_string_helper(void *arg, const char *key, void *item)
{
  char *result_string = arg;
  if (key != NULL) {
    server_player_t* curr = item;
    char *player_string = malloc(100);
    sprintf(player_string, "%c %*d %-3s\n", server_player_getSymbol(curr), 10,
                    server_player_getGoldNumber(curr), server_player_getName(curr));
    strcat(result_string, player_string);
    free(player_string);
  }
}

/**************** send_game_result ****************/
/* At the conclusion of the game, send a result message of
 * how much gold each player has collected.
 *
 * Caller provides:
`*   valid char pointer of the result message to send
 * Note: obtain the result_string by calling game_result_string()
 */
static void
send_game_result(char* result_string)
{
  hashtable_iterate(game->players, result_string, send_game_result_helper);
  // send updates to spectator
  if(game->spectator != NULL) {
    message_send(server_player_getAddress(game->spectator), result_string);
  }
}

/**************** send_game_result_helper ****************/
/* Helper method for send_game_result()
 * Used for iterating through the hashtable of players to send the
 * game result string to each player at the conclusion of the game.
 */
static void
send_game_result_helper(void *arg, const char *key, void *item)
{
  if (key != NULL) {
    char* result_string = arg;
    server_player_t* curr = item;
    message_send(server_player_getAddress(curr), result_string);
  }
}

/**************** game_new ****************/
/* Initalizes a new game_t structure.
 * User provides:
 *      valid char* representing the map map_filename
 * We return a pointer to the newly initalized game structure.
 */
game_t*
game_new(char *map_filename)
{
  if(map_filename == NULL) {
    return NULL;
  }
  game_t* game = count_malloc_assert(sizeof(game_t), "game_t");
  game->playing_game = true;
  game->map_filename = map_filename;
  game->gold_remaining = GoldTotal;
  game->player_number = 0;
  game->n_active_players = 0;
  game->curr_symbol = 'A';
  hashtable_t *ht = hashtable_new(MaxPlayers);
  game->symbol_to_player = set_new();
  game->players = ht;
  game->spectator = NULL;
  game->main_grid = NULL;
  return game;
}

/**************** game_delete ****************/
/* Frees memory used for the game_t structure.
 * User provides:
 *      valid game_t* representing the game structure
 *       we wish to free.
 */
void
game_delete(game_t *game)
{
  if(game != NULL) {
    free(game->map_filename);
    grid_delete(game->main_grid);
    hashtable_delete(game->players, game_delete_helper);
    set_delete(game->symbol_to_player, NULL);
    server_spectator_delete(game->spectator);
    free(game);
  }
}

/**************** game_delete_helper ****************/
/* Helper method for game_delete.
 * Use with hashtable_delete, to delete the player stored
 * as the item in the hashtable.
 */
static void
game_delete_helper(void *item)
{
  if (item != NULL) {
    server_player_delete(item);
  }
}
