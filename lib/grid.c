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
typedef struct point {
  char c;
  bool seen;
  int gold_number;
} point_t;

 typedef struct grid_struct {
   int nR; // number of rows
   int nC; // number of columns
   point_t*** grid; // array of pointers to pointers to points
 } grid_struct_t;


 /**************** global functions ****************/
 /* that is, visible outside this file */
 /* see hashtable.h for comments about exported functions */

 /**************** local functions ****************/
 /* not visible outside this file */
 /* none */

 point_t*
 point_new(char c, bool seen, int gold_number)
 {
   point_t *point = malloc(sizeof(point_t));
   if (point == NULL) {
     return NULL;
   }

   // point->c = malloc(sizeof(char*)); // yes?
   // strcpy(point->c, c);
   point->c = c;
   point->seen = seen;
   point->gold_number = gold_number;
   return point;
 }


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
  grid->grid = count_malloc(grid->nR * sizeof(point_t**));

  for(int i = 0; i < grid->nR; i++) {
    grid->grid[i] = count_malloc(grid->nC * sizeof(point_t*));
  }

  fclose(grid_file);

  return grid;
}

/**************** global functions ****************/
/* that is, visible outside this file */
/* see hashtable.h for comments about exported functions */


int
grid_load(grid_struct_t *grid_struct, char* filename)
{
  FILE *map;
  if ((map = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Unable to load map file.\n");
    return 1;
  }

  char *line;
  int i = 0;

  while ((line = freadlinep(map)) != NULL) {
    for (int j = 0; j < strlen(line); j ++) {
      point_t *p = point_new(line[j], false, 0);
      grid_struct->grid[i][j] = p;
    }
    i++;
    free(line);
  }
  fclose(map);
  return 0;
}

char*
grid_string(grid_struct_t *grid_struct) {
  char grid_text[10000]; // dynamically allocate this
  char *grid_ptr = grid_text;
  bool first_char = true;

  for (int i = 0; i < grid_struct->nR; i++) { // < or <=
    for (int j = 0; j < grid_struct->nC; j++) {
      char c = grid_struct->grid[i][j]->c;
      if (first_char) {
        strcpy(grid_text, &c);
        first_char = false;
      } else {
        strcat(grid_text, &c);
      }
    }
    strcat(grid_text, "\n");
  }

  return grid_ptr;
}


int
grid_print(grid_struct_t *grid_struct)
{
 printf("%s\n", grid_string(grid_struct));
 return 0;
}

int grid_get_nR(grid_struct_t *grid_struct) {
  return grid_struct->nR;
}

int grid_get_nC(grid_struct_t *grid_struct) {
  return grid_struct->nC;
}
