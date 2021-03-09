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
  int gold_picked_up;
  bool active;
  position_t *pos;
  position_t *prev_pos;
  grid_struct_t *grid;
  bool in_passage;
} player_t;

typedef struct game {
  bool playing_game;
  char* map_filename;
  int gold_remaining;
  int gold_pile_number;
  int player_number;
  char curr_symbol;
  grid_struct_t *main_grid;
  hashtable_t *players;
  set_t* symbol_to_port;
  player_t *spectator;
} game_t;

int play_game();

static bool handleMessage(void *arg, const addr_t from, const char *message);
int parse_message(const char *message, addr_t *address);
game_t* game_new(char *map_filename);
player_t* player_new(addr_t address, char *name, char symbol, bool active, position_t *pos);
int add_player(addr_t *address, char* player_name);
void refresh();
static void refresh_helper(void *arg, const char *key, void *item);
int generate_gold(grid_struct_t *grid_struct);

char* game_result_string();
static void game_result_string_helper(void *arg, const char *key, void *item);
void send_game_result(char* result_string);
static void send_game_result_helper(void *arg, const char *key, void *item);

void game_delete(game_t *game);
void spectator_delete(player_t *player);
void player_delete(player_t *player);


// global variable
game_t* game;

// constants (currently commented out the ones we haven't used yet)
static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 26;      // maximum number of players
static const int GoldTotal = 250;      // amount of gold in the game
static const int GoldMinNumPiles = 10; // minimum number of gold piles
static const int GoldMaxNumPiles = 30; // maximum number of gold piles

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
    game->gold_pile_number = generate_gold(game->main_grid);

    grid_print(game->main_grid);

    play_game();
    game_delete(game);

  } else {
    // wrong number of arguments
    fprintf(stderr, "usage: ./server map.txt [seed]\n");
    return 2;
  }
  return 0;
}

int
play_game()
{
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

  if(game->playing_game == false) { // if game has ended, return false
    return true;
  }
  return false;
}

void send_grid(addr_t address) {
  char map_info[256];
  int nC = grid_get_nC(game->main_grid);
  int nR = grid_get_nR(game->main_grid);
  sprintf(map_info, "GRID %d %d", nR, nC);
  message_send(address, map_info);
}

void
send_gold(player_t* player)
{
  int n = player->gold_picked_up;
  int p = player->gold_number;
  int r = game->gold_remaining;
  char gold_info[256];
  sprintf(gold_info, "GOLD %d %d %d", n, p, r);
  message_send(player->address, gold_info);
  player->gold_picked_up = 0;
}


char*
game_result_string()
{
  //sizeof(char) or sizeof(char*)
  // 12 for first part + end of char + new line
  // or no malloc?
  char* result_string = malloc((14 + MaxNameLength * sizeof(char))*(game->player_number+1));
  char* first_line = "QUIT GAME OVER:\n";
  strcpy(result_string, first_line);
  hashtable_iterate(game->players, result_string, game_result_string_helper);
  printf("%s\n", result_string);
  return result_string;
}

static void
game_result_string_helper(void *arg, const char *key, void *item)
{
  char *result_string = arg;
  if (key != NULL) {
    player_t* curr = item;
    char *player_string = malloc(100); // figure this out
    sprintf(player_string, "%c %*d %-3s\n", curr->symbol, 10, curr->gold_number, curr->name);
    strcat(result_string, player_string);
    free(player_string);
  }
}

void
send_game_result(char* result_string)
{
  hashtable_iterate(game->players, result_string, send_game_result_helper);
  // send updates to spectator
  if(game->spectator != NULL) {
    message_send(game->spectator->address, result_string);

  }

}

static void
send_game_result_helper(void *arg, const char *key, void *item)
{
  if (key != NULL) {
    char* result_string = arg;
    player_t* curr = item;
    message_send(curr->address, result_string);
  }
}



void
send_display(player_t* player) {
  // send gold
  send_gold(player);

  // get player grid
  grid_struct_t *grid = player->grid;
  char *string;

  if(player == game->spectator) {
    string = grid_string(game->main_grid);
  } else {
    grid_visibility(grid, player->pos);
    string = grid_string_player(game->main_grid, grid, player->pos);
  }

  // create string to pass grid
  char *display_info = malloc(strlen(string) * sizeof(char*));
  sprintf(display_info, "DISPLAY\n%s", string);
  message_send(player->address, display_info);
  free(string);
  free(display_info);
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

int generate_gold(grid_struct_t *grid_struct) {
  // Number of gold piles to be generated
  int room_spot_number = grid_get_room_spot(grid_struct);
  int localMaxNumPiles;
  if (room_spot_number < GoldMaxNumPiles) {
    localMaxNumPiles = room_spot_number;
  } else {
    localMaxNumPiles = GoldMaxNumPiles;
  }
  // int gold_piles = (rand() % (localMaxNumPiles - GoldMinNumPiles + 1)) + GoldMinNumPiles;

  int gold_piles = 30;

  // Current gold amount generated
  int total_generated = 0;

  // In each loop, generate a new gold pile
  for (int i = 0; i < gold_piles; i++) {

    // TODO: MAKE THE RANDOM NUMBER GENERATION ACTUALLY RANDOM.
    // Get a random number of gold such that the future loops can also get a random number of gold in the range.
    int curr_pile_gold = (rand() % (GoldTotal - total_generated - gold_piles + i + 1) + 1);

    if (gold_piles - i == 1) {
      curr_pile_gold = GoldTotal - total_generated;
    }
    total_generated += curr_pile_gold;

    // Get a random position for the pile
    position_t *curr_pile_pos = generate_position(game->main_grid, '.');

    // Change the point for the pile in the grid
    grid_set_character(game->main_grid, '*', curr_pile_pos);

    // Change the gold amount for the pile in the grid
    grid_set_gold(game->main_grid, curr_pile_gold, curr_pile_pos);

    printf("Gold pile with %d gold at location %d %d\n", curr_pile_gold, pos_get_x(curr_pile_pos), pos_get_y(curr_pile_pos));

    position_delete(curr_pile_pos);
  }
  return gold_piles;
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

  // If the previous point was a . (or player themselves) and the next point is a #, then it's an entry
  if ((prev_c == '.' || isalpha(prev_c)) & (next_c == '#')) {
    return 1;
  }
  // If the previous point was a # (or player themselves) and the next point is a ., then it's an exit
  else if ((prev_c == '#' || isalpha(prev_c)) & (next_c == '.')) {
    return 2;
  }
  // Otherwise it's just normal movement
  else {
    return 0;
  }
}

player_t *get_player(addr_t *address) {
  char portnum1[100];
  sprintf(portnum1, "%d",ntohs((*address).sin_port));
  player_t *curr = hashtable_find(game->players, portnum1);
  return curr;
}

void pickup_gold(player_t *curr, position_t *pos, bool overwrite) {
  if (overwrite) {
    grid_set_character(game->main_grid, '.', curr->pos);
  }
  int gold_picked_up = grid_get_point_gold(game->main_grid, pos_get_x(pos), pos_get_y(pos));
  grid_set_gold(game->main_grid, 0, pos);
  curr->gold_number += gold_picked_up;
  curr->gold_picked_up = gold_picked_up;
  game->gold_remaining -= gold_picked_up;
}


bool move(addr_t *address, int x, int y) {
  player_t *curr = get_player(address);
  // Get the position to the direction of the player
  int new_x = pos_get_x(curr->pos) + x;
  int new_y = pos_get_y(curr->pos) + y;

  // Get the character at the next point
  char c = grid_get_point_c(game->main_grid, new_x, new_y);

  // If the move is valid
  if (c == '.' || isalpha(c) || c == '#' || c == '*') {
    // Swap the player's character with the next point's character
    position_t *new = position_new(new_x, new_y);
    grid_swap(game->main_grid, new, curr->pos);

    // If the next point is a player, then update the next player's position
    if (isalpha(c)) {
      // find the player we swapped positions with
      char* player2_symbol = malloc(sizeof(char) + 1);
      sprintf(player2_symbol, "%c", c);
      player_t* player2 = set_find(game->symbol_to_port, player2_symbol);
      free(player2_symbol);

      // If the player2 is in a passage, swap the passage booleans and the previous positions of both players
      bool temp_passage = player2->in_passage;
      player2->in_passage = curr->in_passage;
      curr->in_passage = temp_passage;

      //reallocate positions for both players
      position_t *prev_pos_curr = position_new(pos_get_x(curr->prev_pos), pos_get_y(curr->prev_pos));
      position_t *curr_pos_curr = position_new(pos_get_x(curr->pos), pos_get_y(curr->pos));
      position_t *prev_pos_p2 = position_new(pos_get_x(player2->prev_pos), pos_get_y(player2->prev_pos));

      if(player2->prev_pos != player2->pos) {
        // free pointer stored in prev pos before updating
        position_delete(player2->prev_pos);
      }
      position_delete(player2->pos);

      if(curr->prev_pos != curr->pos) {
        // free pointer stored in prev pos before updating
        position_delete(curr->prev_pos);
      }
      position_delete(curr->pos);

      player2->prev_pos = prev_pos_curr;
      curr->prev_pos = prev_pos_p2;
      player2->pos = curr_pos_curr;
      curr->pos = new;

    } else {
      // If the next point is a passage, adjust the grid accordingly
      if (c == '#') {
        // If the player is entering the passage, set the previous position to '.', instead of '#'
        if ((point_status(curr->prev_pos, curr->pos) == 1) & !curr->in_passage) {
          grid_set_character(game->main_grid, '.', curr->pos);
          curr->in_passage = true;
        }
      }

      if (c == '.' ) {

        // If the player is exiting a passage, set the previous position to '#' instead of '.'
        if ((point_status(curr->prev_pos, curr->pos) == 2) & curr->in_passage) {
          grid_set_character(game->main_grid, '#', curr->pos);
          curr->in_passage = false;
        }
      }

      // If the next point is a gold_pile, adjust the grid accordingly
      if (c == '*') {
        pickup_gold(curr, new, true);
      }

      // Update the player's position variables to reflect the grid.
      if(curr->prev_pos != curr->pos) {
        // free pointer stored in prev pos before updating
        position_delete(curr->prev_pos);
      }
      position_t *new_prev = position_new(pos_get_x(curr->pos), pos_get_y(curr->pos));
      curr->prev_pos = new_prev;

      // free pointer stored in curr pos before updating
      position_delete(curr->pos);
      curr->pos = new;
    }

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
      spectator_delete(game->spectator);
      game->spectator = new_spectator;
      printf("new spectator joined\n");
    }

    // send spectator a map to draw
    send_grid(*address);
    refresh();

  } else if (strcmp(command, key) == 0) {
    char c = *remainder;
    printf("%c\n", c);
    if (c == 'Q') {
      // send appropriate message depending on if client is spectator or player
      if(game->spectator != NULL && message_eqAddr(game->spectator->address, *address)) {
        message_send(*address, "QUIT Thanks for watching!");
      } else {
        message_send(*address, "QUIT Thanks for playing!");

        // get pointer to player who just quit using their address
        char portnum1[100];
        sprintf(portnum1, "%d",ntohs((*address).sin_port));
        player_t *curr = hashtable_find(game->players, portnum1);
        curr->active = false;

        // remove their symbol from the main grid
        char toReplace = grid_get_point_c(curr->grid, pos_get_x(curr->pos), pos_get_y(curr->pos));
        grid_set_character(game->main_grid, toReplace, curr->pos);
        refresh();
      }
      return 0;
    }

    if (game->spectator == NULL || !message_eqAddr(game->spectator->address, *address)) {

      // switch
      switch (c) {

        // Move to the left by 1
        case 'h':
          move(address, -1, 0);
          break;

        // Move to the left by 1
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


        default:
        // bad key
          message_send(*address, "ERROR unable to understand message");
          break;
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
    send_display(game->spectator);
  }

  // remove these three lines later
  char* main_grid_string = grid_string(game->main_grid);
  printf("MAIN GRID:\n%s", main_grid_string);
  free(main_grid_string);

  if (game->gold_remaining == 0) {
    char* result = game_result_string();
    send_game_result(result);
    free(result);
    game->playing_game = false;

  }

}

static void
refresh_helper(void *arg, const char *key, void *item)
{
  if (key != NULL) {
    player_t* curr = item;
    if(curr->active) {
      printf("curr player name is: %s\n", curr->name);

      // send the player gold information and a display of the grid
      send_display(curr);
    }
  }
}

game_t*
game_new(char *map_filename)
{
  game_t* game = malloc(sizeof(game_t));
  game->playing_game = true;
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
  player->gold_picked_up = 0;
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
  // If there is no empty position, put the player in a gold spot '*'

  position_t *pos;

  if (grid_get_room_spot(game->main_grid) <= game->gold_pile_number) {
    pos = generate_position(game->main_grid, '*');
  } else {
    pos = generate_position(game->main_grid, '.');
  }

  printf("Position is: %d %d\n", pos_get_x(pos), pos_get_y(pos));


  // addr_t new = *address;
  player_t *new_player = player_new(*address, player_name, game->curr_symbol, true, pos);

  // add player to hashtable
  char portnum[100];
  sprintf(portnum, "%d",ntohs((*address).sin_port));
  hashtable_insert(game->players, portnum, new_player);

  char player_symbol[3];
  sprintf(player_symbol, "%c", new_player->symbol);
  set_insert(game->symbol_to_port, player_symbol, new_player);

  // send player the accept message
  char player_info[5];
  sprintf(player_info, "OK %c", new_player->symbol);
  message_send(*address, player_info);

  // add the player to main grid
  grid_set_character(game->main_grid, new_player->symbol, pos);

  // if the player was added into a '*' position, automatically pick up the gold pile.
  pickup_gold(new_player, pos, false);

  // // initialize grid for player
   grid_struct_t *player_grid = grid_struct_new(game->map_filename);
   grid_load(player_grid, game->map_filename, false);
   new_player->grid = player_grid;

  // send the player the grid's information
  send_grid(*address);
  refresh();

  // update current letter and player number
  game->curr_symbol += 1;
  game->player_number += 1;

  return 0;
}

static void
ht_delete_helper(void *item)
{
  if (item != NULL) {
    player_delete(item);
  }
}

void
game_delete(game_t *game) {
  if(game != NULL) {
    free(game->map_filename);
    grid_delete(game->main_grid);
    hashtable_delete(game->players, ht_delete_helper);
    set_delete(game->symbol_to_port, NULL);
    spectator_delete(game->spectator);
    free(game);
  }
}

void
spectator_delete(player_t *player) {
  if(player != NULL) {
    free(player->name);
    free(player);
  }
}

void
player_delete(player_t *player) {
  if(player != NULL) {
    free(player->name);
    free(player->pos);
    free(player->prev_pos);
    grid_delete(player->grid);
    free(player);
  }
}
