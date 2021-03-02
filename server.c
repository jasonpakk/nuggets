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
#include "message.h"
#include "log.h"
#include "hashtable.h"
#include "grid.h"

typedef struct position {
  int x;
  int y;
} position_t;

typedef struct player {
  char *name;
  char symbol;
  addr_t *address;
  int gold_number;
  bool active;
  position_t *pos;
} player_t;

typedef struct game {
  int gold_remaining;
  int player_number;
  char curr_symbol;
  grid_struct_t *grid;
  hashtable_t *players;
  player_t *spectator; // do we want to store in hashmap or seperately like this?
} game_t;

int play_game(int seed);

static bool handleMessage(void *arg, const addr_t from, const char *message);
int parse_message(const char *message, addr_t *address);
game_t* game_new(char *map_filename);
position_t* position_new(int x, int y);
player_t* player_new(addr_t *address, char *name, char symbol, bool active, position_t *pos);

/* The random() and srandom() functions are provided by stdlib,
 * but for some reason not declared by stdlib.h, so we declare here.
 */
long int random(void);
void srandom(unsigned int seed);

// global variable
game_t* game;

int nC;
int nR;

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
    grid_load(main_grid, map_file); // grid_load gives seg fault
    grid_print(main_grid);
    nC = grid_get_nC(main_grid);
    nR = grid_get_nR(main_grid);

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
    srandom((unsigned)time(NULL));
  } else {
    // seed the RNG with provided seed
    srandom(seed);
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

int
parse_message(const char *message, addr_t *address)
{
  // Player to Server Commands
  char *play = "PLAY";
  char *quit = "QUIT";
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
      printf("player being added with name %s and symbol %c\n", remainder, game->curr_symbol);

      // add the player
      position_t *pos = position_new(0,0);
      player_t *new_player = player_new(address, remainder, game->curr_symbol, true, pos);
      // update current letter
      game->curr_symbol = game->curr_symbol + 1;
      game->player_number = game->player_number + 1;
      hashtable_insert(game->players, new_player->name, new_player);



      // send them a map to draw
      char map_info[256];
      snprintf(map_info, sizeof map_info, "GRID %d %d", nC, nR);
      message_send(*address, map_info);

      // send them information about gold_number
      char gold_info[256];
      snprintf(gold_info, sizeof gold_info, "GOLD %d %d %d", 0, new_player->gold_number, game->gold_remaining);
      message_send(*address, gold_info);



    } else {
      // write error message, too many players
    }
  } else if (strcmp(command, spectate) == 0) {
    if (game->spectator == NULL) {
      // no current spectator
      player_t *new_spectator = player_new(address, spectate, '!', false, NULL);
      game->spectator = new_spectator;
      printf("spectator joined\n");


    } else {
      // replace the current spectator

    }

  } else if (strcmp(command, quit) == 0) {

  } else if (strcmp(command, key) == 0) {

  } else {
    printf("unknown message\n");
  }


return 0;
}


game_t*
game_new(char *map_filename)
{
  game_t* game = malloc(sizeof(game_t));
  game->gold_remaining = 0;
  game->player_number = 0;
  game->curr_symbol = 'A';
  hashtable_t *ht = hashtable_new(5); // best number of slots?
  game->players = ht;
  game->spectator = NULL;
  game->grid = NULL;
  return game;
}



player_t*
player_new(addr_t *address, char *name, char symbol, bool active, position_t *pos)
{
  player_t* player = malloc(sizeof(player_t));
  player->name = name;
  player->symbol = symbol;
  player->address = address;
  player->gold_number = 0;
  player->active = active;
  player->pos = pos;
  return player;
}

position_t*
position_new(int x, int y)
{
  position_t* pos = malloc(sizeof(position_t));
  pos->x = x;
  pos->y = y;
  return pos;
}
