/*
 * grid module - storing and modifying the grid
 *
 * Team JEN, Winter 2021
 */

#ifndef __GRID_H
#define __GRID_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

/**************** global types ****************/
typedef struct grid_struct grid_struct_t;  // opaque to users of the module

typedef struct point point_t;

typedef struct position position_t;

/**************** functions ****************/

/* ***************** grid_struct_new ********************** */
/* Create a new empty grid from a map text file
 * Allocate enough memory for the grid to be loaded.
 * We RETURN: pointer to the grid if succesfully created; otherwise we return NULL.
 */
grid_struct_t* grid_struct_new(char* filename);


/* ***************** grid_load ********************** */
/* Load the grid into a pointer to a grid
 * if we are loading grid for game/spectator, seen=true as all the points are seen
 * if we are loading grid for player, seen=false as all points start out false

 * We RETURN: true grid if succesfully created; otherwise we return false.
 */
bool grid_load(grid_struct_t *grid, char* filename, bool seen);

/* ***************** grid_swap ********************** */
/* Swap the characters at two positions in a grid
 */
void grid_swap(grid_struct_t *grid_struct, position_t *pos1, position_t *pos2);

/* ***************** grid_set_character ********************** */
/* Set the character at a grid position
*
 * We RETURN: the previous character in the spot if it exists; otherwise we return '\0'.
 */
char grid_set_character(grid_struct_t *grid_struct, char newChar, position_t *pos);

/* ***************** grid_set_gold ********************** */
/* Set the gold amount at a grid position
 *
 * We RETURN: the previous gold amount in the spot; otherwise we return -1.
 */
int grid_set_gold(grid_struct_t *grid_struct, int newGold, position_t *pos);

/* ***************** grid_get_room_spot ********************** */
/* Get the amount of '.' characters in a grid.
 *
 * We RETURN: the amount of '.' characters in a grid; otherwise we return -1.
 */
int grid_get_room_spot(grid_struct_t *grid_struct);

/* ***************** grid_get_nR ********************** */
/* Get the amount rows in a grid.
 *
 * We RETURN: the amount rows in a grid; otherwise we return -1.
 */
int grid_get_nR(grid_struct_t *grid_struct);

/* ***************** grid_get_nC ********************** */
/* Get the amount columns in a grid.
 *
 * We RETURN: the amount columns in a grid; otherwise we return -1.
 */
int grid_get_nC(grid_struct_t *grid_struct);

/* ***************** grid_get_nC ********************** */
/* Get the character of a point on a grid.
 *
 * We RETURN: the character of a point on a grid; otherwise we return '\0'.
 */
char grid_get_point_c(grid_struct_t *grid_struct, int x, int y);

/* ***************** grid_get_point_gold ********************** */
/* Get the gold at a point on a grid.
 *
 * We RETURN: the gold at a point on a grid; otherwise we return '\0'.
 */
int grid_get_point_gold(grid_struct_t *grid_struct, int x, int y);

/* ***************** grid_string ********************** */
/* Return a printable version of the grid
 *
 * We RETURN: pointer to a character of the whole map if possible; otherwise we return NULL.
 */
char* grid_string(grid_struct_t *grid_struct);

/* ***************** grid_string_player ********************** */
/* Return a printable version of the grid for the player
 * Takes into account the current room the player is in, and also the visibility
 *
 * We RETURN: pointer to a character of the player's grid if possible; otherwise we return NULL.
 */
char* grid_string_player(grid_struct_t *main_grid, grid_struct_t *player_grid, position_t* player_pos);

/* ***************** grid_print ********************** */
/* Print the grid.
 */
void grid_print(grid_struct_t *grid_struct);

/* ***************** grid_visibility ********************** */
/* Calculte the visibility from a position in the grid.
 */
void grid_visibility(grid_struct_t *grid_struct, position_t *pos);

/* ***************** grid_visibility ********************** */
/* Free the grid.
 */
void grid_delete(grid_struct_t *grid_struct);

/* ***************** point_new ********************** */
/* Create a new pointer to a point.
 *
 * We RETURN: pointer to a point if possible; otherwise we return NULL.
 */
point_t* point_new(char c, bool seen, int gold_number);

/* ***************** point_delete ********************** */
/* Free the point.
 */
void point_delete(point_t *point);

/* ***************** position_new ********************** */
/* Create a new pointer to a position.
 *
 * We RETURN: pointer to a position if possible; otherwise we return NULL.
 */
position_t* position_new(int x, int y);

/* ***************** pos_update ********************** */
/* Change the x and y of a position
 */
void pos_update(position_t *pos, int x, int y);

/* ***************** pos_get_x ********************** */
/* Get the x of a position
 *
 * We RETURN: x of a position if possible; otherwise we return -1.
 */
int pos_get_x(position_t *pos);

/* ***************** pos_get_y ********************** */
/* Get the y of a position
 *
 * We RETURN: y of a position if possible; otherwise we return -1.
 */
int pos_get_y(position_t *pos);

/* ***************** position_delete ********************** */
/* Free the position.
 */
void position_delete(position_t *pos);


#endif // __GRID_H
