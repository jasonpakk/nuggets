/*
 * server_player module - storing and modifying each player for use by the server
 *
 * Team JEN, Winter 2021
 */

#ifndef __SERVER_PLAYER_H
#define __SERVER_PLAYER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "message.h"

/**************** global types ****************/
typedef struct server_player server_player_t; // opaque to users of the module

/**************** functions ****************/

// getter functions - access variables of player struct
char* server_player_getName(const server_player_t *player);
char server_player_getSymbol(const server_player_t *player);
addr_t server_player_getAddress(const server_player_t *player);
int server_player_getGoldNumber(const server_player_t *player);
int server_player_getGoldPickedUp(const server_player_t *player);
bool server_player_getActive(const server_player_t *player);
position_t* server_player_getPos(const server_player_t *player);
grid_struct_t* server_player_getGrid(const server_player_t *player);
bool server_player_getInPassage(const server_player_t *player);

// setter functions - change variables of player struct
// returns true on success, returns false on any error
bool server_player_setName(server_player_t *player, char* string);
bool server_player_setSymbol(server_player_t *player, char c);
bool server_player_setAddress(server_player_t *player, addr_t addr);
bool server_player_setGoldNumber(server_player_t *player, int num);
bool server_player_setGoldPickedUp(server_player_t *player, int num);
bool server_player_setActive(server_player_t *player, bool b);
bool server_player_setPos(server_player_t *player, position_t* pos);
bool server_player_setGrid(server_player_t *player, grid_struct_t* grid);
bool server_player_setInPassage(server_player_t *player, bool b);

/**************** server_player_new ****************/
/* Initalizes a new player_t structure.
 * User provides:
 *      valid address to send/recieve messages from the player
 *      valid char* representing the player's name
 *      valid char representing the player's symbol on the game map
 *      valid bool representing whether the player is currently playing
 *      valid position_t* representing position the player is at
 * We return a pointer to the newly initalized player structure.
 */
server_player_t* server_player_new(addr_t address, char *name, char symbol, bool active, position_t *pos);

/**************** server_player_delete ****************/
/* Frees memory used for the player_t structure.
 * User provides:
 *      valid player_t* representing the player structure
 *      we wish to free.
 */
void server_player_delete(server_player_t *player);

 /**************** server_spectator_delete ****************/
 /* Frees memory used to store the spectator.
  * User provides:
  *      valid player_t* representing the spectator
  *      we wish to free.
  */
void server_spectator_delete(server_player_t *player);

#endif // ___SERVER_PLAYER_H
