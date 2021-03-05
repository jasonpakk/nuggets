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

/**************** global types ****************/
typedef struct grid_struct grid_struct_t;  // opaque to users of the module

typedef struct point point_t;

typedef struct position position_t;

/**************** functions ****************/

grid_struct_t* grid_struct_new(char* filename);

int grid_load(grid_struct_t *grid, char* filename, bool seen);

char grid_swap(grid_struct_t *grid_struct, char newChar, position_t *pos);

char* grid_string(grid_struct_t *grid_struct);

int grid_print(grid_struct_t *grid_struct);

int grid_get_nR(grid_struct_t *grid_struct);

int grid_get_nC(grid_struct_t *grid_struct);

char grid_get_point_c(grid_struct_t *grid_struct, int x, int y);

position_t* position_new(int x, int y);

#endif // __GRID_H
