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
 #include "memory.h"

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
grid_struct_new(char *filename)
{
  grid_struct_t *grid = malloc(sizeof(grid_struct_t));
  if (grid == NULL) {
    return NULL;
  }

  FILE *grid_file;
  if ((grid_file = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Unable to load map file.\n");
    return NULL;
  }
  // number of rows is lines in the file
  grid->nR = lines_in_file(grid_file);

  // find the maximum number of columns in the file
  char *line;
  int max = 0;
  while ( (line = freadlinep(grid_file)) != NULL) {
    int currlen = strlen(line);
    if(currlen > max) {
      max = currlen;
    }
    free(line);
  }

  grid->nC = max;
  printf("%d x %d\n", grid->nR, grid->nC);
  grid->grid = count_malloc(grid->nR * grid->nC * sizeof(char *));
  fclose(grid_file);

  return grid;
}

/**************** global functions ****************/
/* that is, visible outside this file */
/* see hashtable.h for comments about exported functions */


int
grid_load(grid_struct_t *grid, char* filename)
{
  FILE *map;
  if ((map = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Unable to load map file.\n");
    return 1;
  }

  char *line;
  int i = 0;
  while ((line = freadlinep(map)) != NULL) {
    grid->grid[i] = line;
    i++;
    // should we copy the string ?
    // (we cant free "line" right here bc the array stores a pointer to it)
  }

  fclose(map);
  return 0;
}

char *grid_string(grid_struct_t *grid_struct) {
  char grid_text[10000];
  char *grid_ptr = grid_text;
  char line_text[200];

  for (int i = 0; i < grid_struct->nR; i++) { // < or <=
    sprintf(line_text, "%s\n", grid_struct->grid[i]);
    if (i == 0) {
      strcpy(grid_text, line_text);
    } else {
      strcat(grid_text, line_text);
    }
  }

  return grid_ptr;
}


int
grid_print(grid_struct_t *grid_struct)
{
 // int i;
 // for (i = 0; i < grid_struct->nR; i++) { // < or <=
 //   printf("%s\n", grid_struct->grid[i]);
 // }
 printf("%s\n", grid_string(grid_struct));
 return 0;
}

int grid_get_nR(grid_struct_t *grid_struct) {
  return grid_struct->nR;
}

int grid_get_nC(grid_struct_t *grid_struct) {
  return grid_struct->nC;
}
