/*
 * grid.c - CS50 grid module
 *
 * see grid.h for more information.
 *
 * Team JEN, Winter 2021
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "grid.h"
 #include "file.h"


/**************** file-local global variables ****************/
/* none */

/**************** local types ****************/
/* none */

/**************** global types ****************/
 typedef struct grid_struct {
   int nR; // number of rows
   int nC; // number of columns
   char** grid; // array of pointers to char pointers
 } grid_struct_t;

 /**************** global functions ****************/
 /* that is, visible outside this file */
 /* see hashtable.h for comments about exported functions */

 /**************** local functions ****************/
 /* not visible outside this file */
 /* none */

grid_struct_t *
grid_struct_new()
{
  grid_struct_t *grid = malloc(sizeof(grid_struct_t));
  if (grid == NULL) {
    return NULL;
  }
  return grid;
}

/**************** global functions ****************/
/* that is, visible outside this file */
/* see hashtable.h for comments about exported functions */


int
grid_load(grid_struct_t *grid, char* filename)
{
  FILE *grid_file;
  if ((grid_file = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Unable to load map file.\n");
    return 1;
  }
  // number of rows is lines in the file
  grid->nR = lines_in_file(grid_file);

  char *line;
  int i = 0;
  while ( (line = freadlinep(grid_file)) != NULL) {
    grid->grid[i] = line;
    i++;
    free(line);
  }
  // number of columns
  grid->nC = strlen(grid->grid[0]); // do we need to +1 for \n ??

  fclose(grid_file);
  return 0;
 }

 int
 grid_print(grid_struct_t *grid_struct)
 {
   int i;
   for (i = 0; i < grid_struct->nR; i++) { // < or <=
     printf("%s\n", grid_struct->grid[i]);

   }
   return 0;
 }
