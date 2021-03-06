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
 #include <math.h>

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

 typedef struct position {
   int x;
   int y;
 } position_t;

 /**************** global functions ****************/
 /* that is, visible outside this file */
 /* see hashtable.h for comments about exported functions */

 /**************** local functions ****************/
 /* not visible outside this file */
 static bool calculate_vision(grid_struct_t *grid_struct, int x1, int y1, int x2, int y2);
 static bool calculate_helper_y(grid_struct_t *grid_struct, double x, int y, bool *found);
 static bool calculate_helper_x(grid_struct_t *grid_struct, int x, double y, bool *found);


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



// if we are loading grid for game/spectator, seen=true as all the points are seen
// if we are loading grid for player, seen=false as all points start out false
int
grid_load(grid_struct_t *grid_struct, char* filename, bool seen)
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
      point_t *p = point_new(line[j], seen, 0);
      grid_struct->grid[i][j] = p;
    }
    i++;
    free(line);
  }
  fclose(map);
  return 0;
}

char
grid_set(grid_struct_t *grid_struct, char newChar, position_t *pos) {
  char oldChar = grid_struct->grid[pos->y][pos->x]->c;

  grid_struct->grid[pos->y][pos->x]->c = newChar;

  return oldChar;
}


void grid_swap(grid_struct_t *grid_struct, position_t *pos1, position_t *pos2) {
  // Get the symbol to swap to
  char symbol2 = grid_get_point_c(grid_struct, pos2->x, pos2->y);

  // If the movement is valid
  // DOESN'T WORK. FIX IT
  if (strcmp(&symbol2, "-") != 0 && strcmp(&symbol2, "|") != 0 && strcmp(&symbol2, "+") != 0) {
    // Change the first position's symbol to the second position's symbol
    char symbol1 = grid_set(grid_struct, symbol2, pos1);

    printf("Swapping %d with %d\n", symbol1, symbol2);


    // Change the second position's symbol to the first position's symbol
    grid_set(grid_struct, symbol1, pos2);
  }
}


char*
grid_string(grid_struct_t *grid_struct) {
  char *grid_text = malloc((grid_struct->nR)*(grid_struct->nC+1)*sizeof(char*));
  bool first_char = true;

  for (int i = 0; i < grid_struct->nR; i++) { // < or <=
    for (int j = 0; j < grid_struct->nC; j++) {
      // Get the character at (i,j)
      char a;
      if(grid_struct->grid[i][j]->seen) {
        a = grid_struct->grid[i][j]->c;
      } else {
        a = ' ';
      }

      if (first_char) {
        strcpy(grid_text, &a);
        first_char = false;
      }
      else {
        strcat(grid_text, &a);
      }
    }
    // Start on a new line for the next iteration of the for loop
    strcat(grid_text, "\n");
  }

  return grid_text;
}

char*
grid_string_player(grid_struct_t *main_grid, grid_struct_t *player_grid, position_t* player_pos) {
  char *grid_text = malloc((main_grid->nR)*(main_grid->nC+1)*sizeof(char*));
  bool first_char = true;

  for (int i = 0; i < main_grid->nR; i++) { // < or <=
    for (int j = 0; j < main_grid->nC; j++) {
      // Get the character at (i,j)
      char a;
      if(pos_get_x(player_pos) == j && pos_get_y(player_pos) == i) {
        a = '@';
      } else if(player_grid->grid[i][j]->seen) {
        a = main_grid->grid[i][j]->c;
      } else {
        a = ' ';
      }

      if (first_char) {
        strcpy(grid_text, &a);
        first_char = false;
      }
      else {
        strcat(grid_text, &a);
      }
    }
    // Start on a new line for the next iteration of the for loop
    strcat(grid_text, "\n");
  }

  return grid_text;
}


int
grid_print(grid_struct_t *grid_struct)
{
 printf("%s\n", grid_string(grid_struct));
 return 0;
}

int
grid_print_player(grid_struct_t *main_grid, grid_struct_t *player_grid, position_t *player_pos)
{
 printf("%s\n", grid_string_player(main_grid, player_grid, player_pos));
 return 0;
}

int
grid_get_nR(grid_struct_t *grid_struct) {
  return grid_struct->nR;
}

int
grid_get_nC(grid_struct_t *grid_struct) {
  return grid_struct->nC;
}

char
grid_get_point_c(grid_struct_t *grid_struct, int x, int y) {
  return grid_struct->grid[y][x]->c;
}

position_t*
position_new(int x, int y)
{
  position_t* pos = malloc(sizeof(position_t));
  pos->x = x;
  pos->y = y;
  return pos;
}

void
pos_update(position_t *pos, int x, int y) {
  pos->x = x;
  pos->y = y;
}

int
pos_get_x(position_t *pos) {
  return pos->x;
}

int
pos_get_y(position_t *pos) {
  return pos->y;
}

void
grid_visibility(grid_struct_t *grid_struct, position_t *pos) {
  // looking through all points in the grid
  for(int r = 0; r < grid_struct->nR; r++) {
    for(int c = 0; c < grid_struct->nC; c++) {
      // if player hasn't seen this point yet
      if(!(grid_struct->grid[r][c]->seen)) {
        // calculate whether they can see this point froom their current position
        if(calculate_vision(grid_struct, pos_get_x(pos), pos_get_y(pos), c, r)) {
          // if so, mark the point's "seen" as true
          grid_struct->grid[r][c]->seen = true;
        }
      }
    }
  }
  //grid_print(grid_struct); // for testing, comment out later
}

static bool
calculate_vision(grid_struct_t *grid_struct, int x1, int y1, int x2, int y2) {
  //printf("calculate: (%d,%d) (%d,%d)\n", x1,y1,x2,y2);
  bool found_non_room = false; // track whether we've run into a non-room space yet

  // 1. Comparing the SAME POINT
  if(x1 == x2 && y1 == y2) {
    return true;
  // 2. UNDEFINED SLOPE (vertical line)
  } else if(x1 == x2) {
    // go through each y in the line segment
    for(int y = 1; y <= abs(y2-y1); y++) {
      int curr_y;   // determining which one to start from (the smaller one)
      if(y1 > y2) {
        curr_y = y1 - y;
      } else {
        curr_y = y1 + y;
      }
      // if not a room spot
      if(grid_struct->grid[curr_y][x1]->c != '.')  {
        // return false if we've already reached a non-room or we've reached solid rock
        if(found_non_room || grid_struct->grid[curr_y][x1]->c == ' ') {
          return false;
        }
        // otherwise, this is the first encounter of a non-room char
        found_non_room = true;
      }
    }
  // 3. SLOPE OF ZERO (horizontal line)
  } else if (y1 == y2) {
    // go through each x in the line segment
    for(int x = 1; x <= abs(x2-x1); x++) {
      int curr_x;   // determining which one to start from (the smaller one)
      if(x1 > x2) {
        curr_x = x1 - x;
      } else {
        curr_x = x1 + x;
      }
      // if not a room spot
      if(grid_struct->grid[y1][curr_x]->c != '.')  {
        // return false if we've already reached a non-room or we've reached solid rock
        if(found_non_room || grid_struct->grid[y1][curr_x]->c == ' ') {
          return false;
        }
        // otherwise, this is the first encounter of a non-room char
        found_non_room = true;
      }
    }
  // 4. LINE SEGMENT
  } else {
    //// PART 1 - Going through each x in the line segment
    double slope = (double) (y2 - y1) / (x2 - x1);  // calculate slope
    if(x1 < x2) {
      // For each x in the line segment:
      double curr_y = y1;
      for(int x = x1+1; x <= x2; x++) {
        curr_y += slope;  // find its respective y value
        // return false if non-room char encountered
        if(!calculate_helper_x(grid_struct, x, curr_y, &found_non_room)) {
          return false;
        }
      }
    } else if (x1 > x2) {
      // For each x in the line segment:
      double curr_y = y1;
      for(int x = x1-1; x >= x2; x--) {
        curr_y = curr_y - slope; // find its respective y value
        // return false if non-room char encountered
        if(!calculate_helper_x(grid_struct, x, curr_y, &found_non_room)) {
          return false;
        }
      }
    }
    //// PART 2 - Going through each y in the line segment
    slope = (double) (x2 - x1) / (y2 - y1);   // calculate slope
    found_non_room = false;                   // reset tracking non-room char
    if(y1 < y2) {
      // For each y in the line segment:
      double curr_x = x1;
      for(int y = y1+1; y <= y2; y++) {
        curr_x += slope;  // find its respective x value
        // return false if non-room char encountered
        if(!calculate_helper_y(grid_struct, curr_x, y, &found_non_room)) {
          return false;
        }
    }
  } else if (y1 > y2) {
      // For each y in the line segment:
      double curr_x = x1;
      for(int y = y1-1; y >= y2; y--) {
        curr_x = curr_x - slope;  // find its respective x value
        // return false if non-room char encountered
        if(!calculate_helper_y(grid_struct, curr_x, y, &found_non_room)) {
          return false;
        }
      }
    }
  }
  // return true, only if it passes all the tests
  return true;
}

static bool
calculate_helper_x(grid_struct_t *grid_struct, int x, double y, bool *found) {
  int c = ceil(fabs(y));  // ceiling function on y value
  int f = floor(fabs(y)); // floor function on y value
  // check boundary cases:
  if(c >= grid_struct->nR) {
    c = c - 1;
  } else if (f < 0) {
    f = 0;
  }
  // if line segment intersects a gridpoint exactly
  if(c == f) {
    // if gridpoint is not a 'room spot'
    if(grid_struct->grid[c][x]->c != '.')  {
      // return false if we've already reached a non-room or we've reached solid rock
      if((*found == true) || grid_struct->grid[c][x]->c == ' ') {
        return false;
      }
      // otherwise, this is the first encounter of a non-room char
      *found = true;
    }
  // if line segment passes between pairs of map gridpoints
  } else {
    // if gridpoint is not a 'room spot'
    if(grid_struct->grid[c][x]->c != '.' && grid_struct->grid[f][x]->c != '.')  {
      // return false if we've already reached a non-room or we've reached solid rock
      if((*found == true) ||
            (((grid_struct->grid[c][x]->c == ' ') && (grid_struct->grid[f][x]->c == ' ')))) {
        return false;
      }
      // otherwise, this is the first encounter of a non-room char
      *found = true;
    }
  }
  return true;
}

static bool
calculate_helper_y(grid_struct_t *grid_struct, double x, int y, bool *found) {
  int c = ceil(fabs(x));   // ceiling function on x value
  int f = floor(fabs(x));   // floor function on x value
  // check boundary cases:
  if(c >= grid_struct->nC) {
    c = c - 1;
  } else if (f < 0) {
    f = 0;
  }
  // if line segment intersects a gridpoint exactly
  if(c == f) {
    // if gridpoint is not a 'room spot'
    if(grid_struct->grid[y][c]->c != '.')  {
      // return false if we've already reached a non-room or we've reached solid rock
      if((*found == true) || grid_struct->grid[y][c]->c == ' ') {
        return false;
      }
      // otherwise, this is the first encounter of a non-room char
      *found = true;
    }
  } else {
    // if gridpoint is not a 'room spot'
    if(grid_struct->grid[y][c]->c != '.' && grid_struct->grid[y][f]->c != '.')  {
      // return false if we've already reached a non-room or we've reached solid rock
      if((*found == true) ||
            (((grid_struct->grid[y][c]->c == ' ') && (grid_struct->grid[y][f]->c == ' ')))) {
        return false;
      }
      // otherwise, this is the first encounter of a non-room char
      *found = true;
    }
  }
  return true;
}
