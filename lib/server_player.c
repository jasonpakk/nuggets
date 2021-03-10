/*
 * server_player.c - player module to stores data about one player in a nuggets game
 *
 * see server_player.h for more information.
 *
 * Team JEN, Winter 2021
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "grid.h"
 #include "memory.h"
 #include "message.h"

/**************** file-local global variables ****************/
/* none */

/**************** local types ****************/
/* none */

/**************** global types ****************/
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

/**************** global functions ****************/
/* that is, visible outside this file */
/* see server_player.h for comments about exported functions */

/**************** local functions ****************/
/* not visible outside this file */

/* *********************************************************************** */
/* getter methods */
char* server_player_getName(const server_player_t *player) {
  return player ? player->name : 0;
}
char server_player_getSymbol(const server_player_t *player) {
  return player ? player->symbol : '\0';
}
addr_t server_player_getAddress(const server_player_t *player) {
  return player->address;
}
int server_player_getGoldNumber(const server_player_t *player) {
  return player ? player->gold_number : 0;
}
int server_player_getGoldPickedUp(const server_player_t *player) {
  return player ? player->gold_picked_up : 0;
}
bool server_player_getActive(const server_player_t *player) {
  return player->active;
}
position_t* server_player_getPos(const server_player_t *player) {
  return player ? player->pos : NULL;
}
grid_struct_t* server_player_getGrid(const server_player_t *player) {
  return player ? player->grid : NULL;
}
bool server_player_getInPassage(const server_player_t *player) {
  return player->in_passage;
}

/* *********************************************************************** */
/* setter methods */
void server_player_setName(server_player_t *player, char* string) {
  player->name = string;
}
void server_player_setSymbol(server_player_t *player, char c) {
  player->symbol = c;
}
void server_player_setAddress(server_player_t *player, addr_t addr) {
  player->address = addr;
}
void server_player_setGoldNumber(server_player_t *player, int num) {
  player->gold_number = num;
}
void server_player_setGoldPickedUp(server_player_t *player, int num) {
  player->gold_picked_up = num;
}
void server_player_setActive(server_player_t *player, bool b) {
  player->active = b;
}
void server_player_setPos(server_player_t *player, position_t* pos) {
  player->pos = pos;
}
void server_player_setGrid(server_player_t *player, grid_struct_t* grid) {
  player->grid = grid;
}
void server_player_setInPassage(server_player_t *player, bool b) {
  player->in_passage = b;
}

/**************** server_player_new ****************/
/* see server_player.h for documentation */
server_player_t*
server_player_new(addr_t address, char *name, char symbol, bool active, position_t *pos)
{
  server_player_t* player = count_malloc_assert(sizeof(server_player_t), "server_player_t");
  player->name = count_malloc_assert(strlen(name)+1, "player_t name string");
  strcpy(player->name, name);
  player->symbol = symbol;
  player->gold_picked_up = 0;
  player->address = address;
  player->gold_number = 0;
  player->active = active;
  player->pos = pos;
  player->grid = NULL;
  player->in_passage = false;
  return player;
}

/**************** server_player_delete ****************/
/* see server_player.h for documentation */
void
server_player_delete(server_player_t *player)
{
  if(player != NULL) {
    free(player->name);
    free(player->pos);
    grid_delete(player->grid);
    free(player);
  }
}

/**************** server_spectator_delete ****************/
/* see server_player.h for documentation */
void
server_spectator_delete(server_player_t *player)
{
  if(player != NULL) {
    free(player->name);
    free(player);
  }
}
