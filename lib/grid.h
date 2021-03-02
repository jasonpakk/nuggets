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

/**************** functions ****************/

grid_struct_t* grid_struct_new(char* filename);

int grid_load(grid_struct_t *grid, char* filename);

char* grid_string(grid_struct_t *grid_struct);

int grid_print(grid_struct_t *grid_struct);

int grid_get_nR(grid_struct_t *grid_struct);

int grid_get_nC(grid_struct_t *grid_struct);

#endif // __GRID_H
