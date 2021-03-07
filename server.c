/*
 * server.c      Team JEN      March, 2021
 *
 * server.c    View README.md for functionality
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include "message.h"
#include "log.h"
#include "hashtable.h"
#include "set.h"
#include "grid.h"

typedef struct player {
  char *name;
  char symbol;
  addr_t address;
  int gold_number;
  bool active;
  position_t *pos;
  position_t *prev_pos;
  grid_struct_t *grid;
  bool in_passage;
} player_t;

typedef struct game {
  char* map_filename;
  int gold_remaining;
  int player_number;
  char curr_symbol;
  grid_struct_t *main_grid;
  hashtable_t *players;
  set_t* symbol_to_port;
  player_t *spectator; // STORE IN HASHTABLE, keep bool of if there is a spectator // have special symbol for spectator
} game_t;

int play_game(int seed);

static bool handleMessage(void *arg, const addr_t from, const char *message);
int parse_message(const char *message, addr_t *address);
game_t* game_new(char *map_filename);
player_t* player_new(addr_t address, char *name, char symbol, bool active, position_t *pos);
int add_player(addr_t *address, char* player_name);
void refresh();
static void refresh_helper(void *arg, const char *key, void *item);

// global variable
game_t* game;

// constants (currently commented out the ones we haven't used yet)
//static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 26;      // maximum number of players
static const int GoldTotal = 250;      // amount of gold in the game
//static const int GoldMinNumPiles = 10; // minimum number of gold piles
//static const int GoldMaxNumPiles = 30; // maximum number of gold piles

int
main(const int argc, const char *argv[])
{
  if (argc == 2 || argc == 3) {
    char* map_file = (char*)malloc(1+ strlen(argv[1])*sizeof(char));
    strcpy(map_file, argv[1]);

    // initialize game
    game = game_new(map_file);

    // initialize first grid
    grid_struct_t *main_grid = grid_struct_new(map_file);
    // all points are seen
    grid_load(main_grid, map_file, true);
    game->main_grid = main_grid;
    grid_print(game->main_grid);


    // no seed
    if (argc == 2) {
      // generate a seed
        play_game(0);
    } else {
      //seed provided, scan it in and check it is positive
      int seed = atoi(argv[2]);
      if (seed <= 0) {
        fprintf(stderr, "the seed must be a positive integer.\n");
        return 1;
      }
      play_game(seed);
    }
  } else {
    // wrong number of arguments
    fprintf(stderr, "usage: ./server map.txt [seed]\n");
    return 2;
  }
}

int
play_game(int seed)
{
  if (seed == 0) {
    // seed the RNG (random-number generator) with the time of day
    srand(getpid());
  } else {
    // seed the RNG with provided seed
    srand(seed);
  }
  addr_t other; // address of the other side of this communication (init below)

  log_init(stderr);
  // initialize the message module
  int ourPort = message_init(stderr);
  if (ourPort == 0) {
    return 2; // failure to initialize message module
  }
  printf("waiting on port %d for contact....\n", ourPort);
  other = message_noAddr(); // no correspondent yet
  bool ok = message_loop(&other, 0, NULL, NULL, handleMessage);
  message_done();
  return ok;
}

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
  printf("[%s@%05d]: %s\n",
         inet_ntoa(from.sin_addr), // IP address of the sender
         ntohs(from.sin_port),     // port number of the sender
         message);                 // message from the sender
  parse_message(message, otherp);

  fflush(stdout);
  return false;
}

void send_grid(addr_t address) {
  char map_info[256];
  int nC = grid_get_nC(game->main_grid);
  int nR = grid_get_nR(game->main_grid);
  sprintf(map_info, "GRID %d %d", nC, nR);
  message_send(address, map_info);
}

void
send_gold(addr_t address, int n, int p, int r) {
  char gold_info[256];
  sprintf(gold_info, "GOLD %d %d %d", n, p, r);
  message_send(address, gold_info);
}

void
send_display(player_t* player) {
  // get player grid
  grid_struct_t *grid = player->grid;
  grid_visibility(grid, player->pos);

  // create string to pass grid
  char *string = grid_string_player(game->main_grid, grid, player->pos);
  char *display_info = malloc(strlen(string) * sizeof(char*));
  sprintf(display_info, "DISPLAY\n%s", string);
  message_send(player->address, display_info);
}

position_t*
generate_position(grid_struct_t *grid_struct, char valid_symbol) {
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

void nameprint(FILE *fp, const char *key, void *item)
{
  player_t *name = item;
  if (name == NULL) {
    fprintf(fp, "(null)");
  }
  else {
    char portnum2[100];
    sprintf(portnum2, "%d",ntohs((name->address).sin_port));
    fprintf(fp, "(%s,%s)", key, portnum2);
  }
}

int point_status(position_t *prev_pos, position_t *next_pos) {

  int prev_x = pos_get_x(prev_pos);
  int prev_y = pos_get_y(prev_pos);
  char prev_c = grid_get_point_c(game->main_grid, prev_x, prev_y);

  int next_x = pos_get_x(next_pos);
  int next_y = pos_get_y(next_pos);
  char next_c = grid_get_point_c(game->main_grid, next_x, next_y);

  // If the previous point was a . and the next point is a #, then it's an entry
  if ((prev_c == '.') & (next_c == '#')) {
    return 1;
  }
  // If the previous point was a # and the next point is a ., then it's an exit
  else if ((prev_c == '#') & (next_c == '.')) {
    return 2;
  }
  // Otherwise it's just normal movement
  else {
    return 0;
  }
}

bool move(addr_t *address, int x, int y) {
  // Get the player
  char portnum1[100];
  sprintf(portnum1, "%d",ntohs((*address).sin_port));
  player_t *curr = hashtable_find(game->players, portnum1);
  // Get the position to the direction of the player
  int new_x = pos_get_x(curr->pos) + x;
  int new_y = pos_get_y(curr->pos) + y;

  // Get the character at the next point
  char c = grid_get_point_c(game->main_grid, new_x, new_y);

  // If the move is valid
  if (c == '.' || isalpha(c) || c == '#') {
    // Swap the player's character with the next point's character
    position_t *new = position_new(new_x, new_y);
    grid_swap(game->main_grid, new, curr->pos);

    // If the next point is a player, then update the next player's position
    if (isalpha(c)) {
      char* player2_symbol = malloc(sizeof(char) + 1);
      sprintf(player2_symbol, "%c", c);

      player_t* player2 = set_find(game->symbol_to_port, player2_symbol);
      player2->prev_pos = player2->pos;
      pos_update(player2->pos, pos_get_x(curr->pos), pos_get_y(curr->pos));
      set_print(game->symbol_to_port, stdout, nameprint);

      // If the player2 is in a passage, swap the passage booleans and the previous positions of both players
      bool temp_passage = player2->in_passage;
      player2->in_passage = curr->in_passage;
      curr->in_passage = temp_passage;

      position_t *temp_prev_pos = player2->prev_pos;
      player2->prev_pos = curr->prev_pos;
      curr->prev_pos = temp_prev_pos;

    }



    // If the next point is a passage, adjust the grid accordingly
    if (c == '#') {
      // If the player is entering the passage, set the previous position to '.', instead of '#'
      if ((point_status(curr->prev_pos, curr->pos) == 1) & !curr->in_passage) {
        grid_set(game->main_grid, '.', curr->pos);
        curr->in_passage = true;
      }
    }

    if (c == '.' ) {

      // If the player is exiting a passage, set the previous position to '#' instead of '.'
      if ((point_status(curr->prev_pos, curr->pos) == 2) & curr->in_passage) {
        grid_set(game->main_grid, '#', curr->pos);
        curr->in_passage = false;
      }
    }

    // Update the player's position variables to reflect the grid.
    curr->prev_pos = curr->pos;
    curr->pos = new;

    refresh();
    return true;
  }
  return false;
}

int
parse_message(const char *message, addr_t *address)
{
  // Player to Server Commands
  char *play = "PLAY";
  char *key = "KEY";
  char *spectate = "SPECTATE";
  // add defensive programming
  // is this bad practice to cast?
  char *command = (char*) message;
  char *remainder = (char*) message;
  while (!isspace(*remainder)) {
    remainder++;
  }
  // found end of word, put end of string character
  *remainder = '\0';
  //increment remainter to find start of the rest of message
  remainder++;

  // Server to Player commands;
  if (strcmp(command, play) == 0) {
        // add player
    if (game->player_number <= MaxPlayers && strlen(remainder) > 0) {
      add_player(address, remainder);
    } else {
      if (game->player_number > MaxPlayers) {
        // write error message, too many players
        message_send(*address, "QUIT Game is full: no more players can join.");
      } else if(strlen(remainder) == 0) {
        // write error message, name not provided
        message_send(*address, "QUIT Sorry - you must provide player's name.");
      }
    }
  } else if (strcmp(command, spectate) == 0) {
    // initalize new spectator
    addr_t new = *address;
    player_t *new_spectator = player_new(new, spectate, '!', false, NULL);
    new_spectator->grid = game->main_grid;

    if (game->spectator == NULL) {
      // no current spectator
      game->spectator = new_spectator;
      printf("spectator joined\n");
    } else {
      // otherwise, replace the current spectator
      message_send(game->spectator->address, "QUIT You have been replaced by a new spectator.");
      //LATER: call PLAYER_DELETE to free memory of previous spectator
      game->spectator = new_spectator;
      printf("new spectator joined\n");
    }

    // send spectator a map to draw
    send_grid(*address);
    refresh();

  } else if (strcmp(command, key) == 0) {
    // if QUIT key pressed
    if (strcmp(remainder, "Q") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator != NULL && message_eqAddr(game->spectator->address, *address)) {
        message_send(*address, "QUIT Thanks for watching!");
      } else {
        message_send(*address, "QUIT Thanks for playing!");
      }
    }

    // Move left
    if (strcmp(remainder, "h") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        // Move to the left by 1
        move(address, -1, 0);
      }
    }

    // Move left (until not possible)
    if (strcmp(remainder, "H") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        // Move to the left by 1
        while(move(address, -1, 0));
      }
    }

    // Move right
    if (strcmp(remainder, "l") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        move(address, 1, 0);
      }
    }

    // Move right (until not possible)
    if (strcmp(remainder, "L") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        while(move(address, 1, 0));
      }
    }

    // Move up
    if (strcmp(remainder, "k") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        move(address, 0, -1);
      }
    }

    // Move up (until not possible)
    if (strcmp(remainder, "K") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        while(move(address, 0, -1));
      }
    }

    // Move down
    if (strcmp(remainder, "j") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        move(address, 0, 1);
      }
    }

    // Move down (until not possible)
    if (strcmp(remainder, "J") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        while(move(address, 0, 1));
      }
    }

    // move diagonally up and left
    if (strcmp(remainder, "y") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        move(address, -1, -1);
      }
    }

    // move diagonally up and left (until not possible)
    if (strcmp(remainder, "Y") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        while(move(address, -1, -1));
      }
    }

    // move diagonally up and right
    if (strcmp(remainder, "u") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        move(address, 1, -1);
      }
    }

    // move diagonally up and right (until not possible)
    if (strcmp(remainder, "U") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        while(move(address, 1, -1));
      }
    }

    // move diagonally down and left
    if (strcmp(remainder, "b") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        move(address, -1, 1);
      }
    }

    // move diagonally down and left (until not possible)
    if (strcmp(remainder, "B") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        while(move(address, -1, 1));
      }
    }

    // move diagonally down and right
    if (strcmp(remainder, "n") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        move(address, 1, 1);
      }
    }

    // move diagonally down and right (until not possible)
    if (strcmp(remainder, "N") == 0) {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {
        while(move(address, 1, 1));
      }
    }

  } else {
    message_send(*address, "ERROR unable to understand message");
  }

  return 0;
}

void
refresh()
{
  hashtable_iterate(game->players, NULL, refresh_helper);
  // send updates to spectator
  if(game->spectator != NULL) {
    send_gold(game->spectator->address, 0, 0, game->gold_remaining);
    send_display(game->spectator);
  }
  printf("MAIN GRID:\n%s", grid_string(game->main_grid));
}

// hashtable_iterate helper function for adding a char to a grid
// call hashtable_iterate(game->players, char, add_char)
// need to pass a position and a char (?)
// static void
// add_char()
// {
//
// }

static void
refresh_helper(void *arg, const char *key, void *item)
{
  if (key != NULL) {
    player_t* curr = item;
    printf("curr player name is: %s\n", curr->name);
    // send the player gold information (UPDATE LATER)
    send_gold(curr->address, 0, curr->gold_number, game->gold_remaining);

    // send the player a display of the grid
    send_display(curr);
  }
}

game_t*
game_new(char *map_filename)
{
  game_t* game = malloc(sizeof(game_t));
  game->map_filename = map_filename;
  game->gold_remaining = GoldTotal;
  game->player_number = 0;
  game->curr_symbol = 'A';
  hashtable_t *ht = hashtable_new(MaxPlayers);
  game->symbol_to_port = set_new();
  game->players = ht;
  game->spectator = NULL;
  game->main_grid = NULL;
  return game;
}

player_t*
player_new(addr_t address, char *name, char symbol, bool active, position_t *pos)
{
  player_t* player = malloc(sizeof(player_t));
  player->name = malloc(strlen(name)+1);
  strcpy(player->name, name);
  player->symbol = symbol;
  player->address = address;
  player->gold_number = 0;
  player->active = active;
  player->pos = pos;
  player->grid = NULL;
  player->in_passage = false;
  player->prev_pos = pos;
  return player;
}

// find a position
// create a new player
// add to hashtable
// initialize player grid
// put player in their grid and in all other player grids

// send player gold info, grid info and the grid string

int
add_player(addr_t *address, char* player_name)
{
  printf("player being added with name %s and symbol %c\n", player_name, game->curr_symbol);

  // Find a position to put the player in and create a new player
  position_t *pos = generate_position(game->main_grid, '.');
  addr_t new = *address;
  player_t *new_player = player_new(new, player_name, game->curr_symbol, true, pos);

  // add player to hashtable
  char portnum[100];
  sprintf(portnum, "%d",ntohs((*address).sin_port));
  hashtable_insert(game->players, portnum, new_player);

  char player_symbol[3];
  sprintf(player_symbol, "%c", new_player->symbol);
  set_insert(game->symbol_to_port, player_symbol, new_player);
  set_print(game->symbol_to_port, stdout, nameprint);

  // send player the accept message
  char player_info[5];
  sprintf(player_info, "OK %c", new_player->symbol);
  message_send(*address, player_info);

  // add the player to main grid
  grid_set(game->main_grid, new_player->symbol, pos);
  // grid_print(game->main_grid);

  // delete pos here or in grid swap ?
  //
  // // initialize grid for player
   grid_struct_t *player_grid = grid_struct_new(game->map_filename);
   grid_load(player_grid, game->map_filename, false);
   new_player->grid = player_grid;

  //new_player->grid = game->main_grid;

  // send the player the grid's information
  send_grid(*address);
  refresh();

  // update current letter and player number
  game->curr_symbol = game->curr_symbol + 1;
  game->player_number = game->player_number + 1;

  return 0;
}
