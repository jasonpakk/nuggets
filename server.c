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
#include "grid.h"

typedef struct player {
  char *name;
  char symbol;
  addr_t address; // should this be ponter to address ?
  int gold_number;
  bool active;
  position_t *pos;
  grid_struct_t *grid;
} player_t;

typedef struct game {
  char* map_filename;
  int gold_remaining;
  int player_number;
  char curr_symbol;
  grid_struct_t *main_grid;
  hashtable_t *players;
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

  // create string to pass grid
  char *string = grid_string(grid);
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
    rand_x = rand() % grid_get_nR(grid_struct);
    rand_y = rand() % grid_get_nC(grid_struct);

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
    if (game->player_number < 26) {
      add_player(address, remainder);

    } else {
      // write error message, too many players
    }
  } else if (strcmp(command, spectate) == 0) {
    if (game->spectator == NULL) {
      // no current spectator
      player_t *new_spectator = player_new(*address, spectate, '!', false, NULL);
      game->spectator = new_spectator;
      new_spectator->grid = game->main_grid;
      printf("spectator joined\n");

      // send them a map to draw
      send_grid(*address);
      send_display(new_spectator);

    } else {
      // replace the current spectator


    }

  } else if (strcmp(command, key) == 0) {
    if (strcmp(remainder, "Q")) {
      message_send(*address, "QUIT Thanks for playing!");
    }

  } else {
    printf("unknown message\n");
  }


return 0;
}

void
refresh()
{
  hashtable_iterate(game->players, NULL, refresh_helper);

  if(game->spectator != NULL) {
    send_display(game->spectator);
  }
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
  game->gold_remaining = 0;
  game->player_number = 0;
  game->curr_symbol = 'A';
  hashtable_t *ht = hashtable_new(5); // best number of slots?
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
  grid_print(game->main_grid);

  // Find a position to put the player in and create a new player
  position_t *pos = generate_position(game->main_grid, '.');
  player_t *new_player = player_new(*address, player_name, game->curr_symbol, true, pos);

  // add player to ht and update current letter and player number
  game->curr_symbol = game->curr_symbol + 1;
  game->player_number = game->player_number + 1;
  char portnum[100];
  sprintf(portnum, "%d",ntohs((*address).sin_port));
  hashtable_insert(game->players, portnum, new_player);

  // add the player to main grid
  grid_swap(game->main_grid, new_player->symbol, pos);
  // delete pos here or in grid swap ?

  // initialize grid for player
  grid_struct_t *player_grid = grid_struct_new(game->map_filename);

  grid_load(player_grid, game->map_filename, false);
  new_player->grid = player_grid;


  // WILL SPECTATOR GRID JUST BE MAIN GRID ?????


  // send the player the grid's information
  send_grid(*address);


  // send the player gold information
  //send_gold(address, 0, new_player->gold_number, game->gold_remaining);

  // send the player a display of the grid
  //send_display(address);
  refresh();
  return 0;
}
