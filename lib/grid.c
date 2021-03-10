/*
 * grid.c - Grid module to represent the game map for the nuggets game
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
  char c;     // char at the point
  bool seen_before;   // has the player ever seen this point before
  bool visible_now;   // is the point visible from a player's current position
  int gold_number;  // amount of gold (gold piles)
} point_t;

 typedef struct grid_struct {
   int nR; // number of rows
   int nC; // number of columns
   int room_spot; // number of room spots
   point_t*** grid; // 2d array of pointers to points
 } grid_struct_t;

 typedef struct position {
   int x;   // x coord
   int y;   // y coord
 } position_t;

/**************** global functions ****************/
/* that is, visible outside this file */
/* see grid.h for comments about exported functions */

/**************** local functions ****************/
/* not visible outside this file */
static void calculate_vision_passage(grid_struct_t *grid_struct, int x, int y);
static bool calculate_vision(grid_struct_t *grid_struct, int x1, int y1, int x2, int y2);
static bool calculate_helper_y(grid_struct_t *grid_struct, double x, int y);
static bool calculate_helper_x(grid_struct_t *grid_struct, int x, double y);

/**************** global functions ****************/
/* that is, visible outside this file */

/**************** grid_struct_new ****************/
/* see grid.h for documentation */
grid_struct_t *
grid_struct_new(char *filename)
{
  // allocate memory; error message on error
  grid_struct_t *grid = count_malloc_assert(sizeof(grid_struct_t), "grid_struct_t");
  grid->room_spot = 0;

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

  // allocate memory for the 2d array
  grid->grid = count_malloc_assert(grid->nR * sizeof(point_t**), "grid_struct_t");
  for(int i = 0; i < grid->nR; i++) {
    grid->grid[i] = count_malloc_assert(grid->nC * sizeof(point_t*), "grid_struct_t");
  }

  fclose(grid_file);
  return grid;
}

/**************** grid_load ****************/
/* see grid.h for documentation */
bool
grid_load(grid_struct_t *grid_struct, char* filename, bool seen)
{
  // check parameters
  if(grid_struct == NULL || filename == NULL) {
    return false;
  }
  // try reading from file; return false on error
  FILE *map;
  if ((map = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Unable to load map file.\n");
    return false;
  }
  // loop through each char in the map file
  char *line;
  int i = 0;
  while ((line = freadlinep(map)) != NULL) {
    for (int j = 0; j < strlen(line); j ++) {
      // instantiate a point using the char
      point_t *p = point_new(line[j], seen, 0);
      // mark it if the char is a room spot
      if (line[j] == '.') {
        grid_struct->room_spot = grid_struct->room_spot +1;
      }
      // add to grid array
      grid_struct->grid[i][j] = p;
    }
    i++;
    free(line);
  }
  fclose(map);
  return true;
}

/**************** grid_swap ****************/
/* see grid.h for documentation */
void
grid_swap(grid_struct_t *grid_struct, position_t *pos1, position_t *pos2)
{
  // check parameters
  if(grid_struct == NULL || pos1 == NULL || pos2 == NULL) {
    return;
  }
  // ensure pos1 doesn't go out of bounds
  if (pos_get_x(pos1) < 0 || pos_get_x(pos1) >= grid_struct->nC
          || pos_get_y(pos1) < 0 || pos_get_y(pos1) >= grid_struct->nR) {
    return;
  }
  // ensure pos2 doesn't go out of bounds
  if (pos_get_x(pos2) < 0 || pos_get_x(pos2) >= grid_struct->nC
          || pos_get_y(pos2) < 0 || pos_get_y(pos2) >= grid_struct->nR) {
    return;
  }

  // Get the symbol to swap to
  char symbol2 = grid_get_point_c(grid_struct, pos2->x, pos2->y);
  // If the movement is valid
  if (strcmp(&symbol2, "-") != 0 && strcmp(&symbol2, "|") != 0 && strcmp(&symbol2, "+") != 0) {
    // Change the first position's symbol to the second position's symbol
    char symbol1 = grid_set_character(grid_struct, symbol2, pos1);
    // Change the second position's symbol to the first position's symbol
    grid_set_character(grid_struct, symbol1, pos2);
  }
}

/**************** grid_set_character ****************/
/* see grid.h for documentation */
char
grid_set_character(grid_struct_t *grid_struct, char newChar, position_t *pos)
{
  // check parameters
  if(grid_struct == NULL || pos == NULL) {
    return '\0';
  }
  // ensure pos doesn't go out of bounds
  if (pos_get_x(pos) < 0 || pos_get_x(pos) >= grid_struct->nC
          || pos_get_y(pos) < 0 || pos_get_y(pos) >= grid_struct->nR) {
    return '\0';
  }

  // store a copy of the current char
  char oldChar = grid_struct->grid[pos->y][pos->x]->c;
  // update with new char
  grid_struct->grid[pos->y][pos->x]->c = newChar;
  return oldChar;
}

/**************** grid_set_gold ****************/
/* see grid.h for documentation */
int
grid_set_gold(grid_struct_t *grid_struct, int newGold, position_t *pos)
{
  // check parameters
  if(grid_struct == NULL || pos == NULL) {
    return -1;
  }
  // ensure pos doesn't go out of bounds
  if (pos_get_x(pos) < 0 || pos_get_x(pos) >= grid_struct->nC
          || pos_get_y(pos) < 0 || pos_get_y(pos) >= grid_struct->nR) {
    return -1;
  }

  // store a copy of the current amount of gold
  int oldGold = grid_struct->grid[pos->y][pos->x]->gold_number;
  // update with new amount of gold
  grid_struct->grid[pos->y][pos->x]->gold_number = newGold;
  return oldGold;
}

/**************** grid_get_room_spot ****************/
/* see grid.h for documentation */
int
grid_get_room_spot(grid_struct_t *grid_struct)
{
  if(grid_struct == NULL) { // check parameter
    return -1;
  }
  return grid_struct->room_spot;
}

/**************** grid_get_nR ****************/
/* see grid.h for documentation */
int
grid_get_nR(grid_struct_t *grid_struct)
{
  if( grid_struct == NULL ) { // check parameters
    return -1;
  }
  return grid_struct->nR;
}

/**************** grid_get_nC ****************/
/* see grid.h for documentation */
int
grid_get_nC(grid_struct_t *grid_struct)
{
  if( grid_struct == NULL ) { // check parameters
    return -1;
  }
  return grid_struct->nC;
}

/**************** grid_get_point_c ****************/
/* see grid.h for documentation */
char
grid_get_point_c(grid_struct_t *grid_struct, int x, int y)
{
  // check parameters
  if (x < 0 || x >= grid_struct->nC || y < 0 || y >= grid_struct->nR) {
    return '\0';
  }
  return grid_struct->grid[y][x]->c;
}

/**************** grid_get_point_gold ****************/
/* see grid.h for documentation */
int
grid_get_point_gold(grid_struct_t *grid_struct, int x, int y)
{
  // check parameters
  if (grid_struct == NULL || x < 0 || x >= grid_struct->nC || y < 0 || y >= grid_struct->nR) {
    return -1;
  }
  return grid_struct->grid[y][x]->gold_number;
}

/**************** grid_string ****************/
/* see grid.h for documentation */
char*
grid_string(grid_struct_t *grid_struct)
{
  // check parameters
  if(grid_struct == NULL) {
    return NULL;
  }

  // allocate memory; error message on error
  char *grid_text = count_malloc_assert((grid_struct->nR)*(grid_struct->nC+1)*sizeof(char*),
                                              "grid as string");
  bool first_char = true; // track if we are reading the first char
  for (int i = 0; i < grid_struct->nR; i++) {
    for (int j = 0; j < grid_struct->nC; j++) {
      // Get the character at row = i, column = j
      char a;
      if(grid_struct->grid[i][j]->seen_before) {
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

/**************** grid_string_player ****************/
/* see grid.h for documentation */
char*
grid_string_player(grid_struct_t *main_grid, grid_struct_t *player_grid, position_t* player_pos)
{
  // check parameters
  if(main_grid == NULL || player_grid == NULL || player_pos == NULL) {
    return NULL;
  }

  // allocate memory; error message on error
  char *grid_text = count_malloc_assert((main_grid->nR)*(main_grid->nC+1)*sizeof(char*),
                                            "grid as string");
  bool first_char = true; // track if we are reading the first char
  // loop through points in grid
  for (int i = 0; i < main_grid->nR; i++) {
    for (int j = 0; j < main_grid->nC; j++) {
      // Get the character at row = i, column = j
      char a;
      // if this is the current player, display as '@'
      if(pos_get_x(player_pos) == j && pos_get_y(player_pos) == i) {
        a = '@';
      } else if(player_grid->grid[i][j]->seen_before) {
          // if a gold spot
          if(main_grid->grid[i][j]->c == '*') {
            // print the gold only if we can see it in our current position
            if(player_grid->grid[i][j]->visible_now) {
              a = '*';
            // otherwise, print a room spot
            } else {
              a = '.';
            }
          // if not a gold spot, print the char at the point
          } else {
              a = main_grid->grid[i][j]->c;
          }
      // if the point is not visible, print an empty space
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

/**************** grid_print ****************/
/* see grid.h for documentation */
void
grid_print(grid_struct_t *grid_struct)
{
  if(grid_struct == NULL ) { // check parameters
    return;
  }
  // obtain grid as a string and print
  char* toPrint = grid_string(grid_struct);
  printf("%s\n", toPrint);
  free(toPrint);
}

/**************** grid_visibility ****************/
/* see grid.h for documentation */
void
grid_visibility(grid_struct_t *grid_struct, position_t *pos)
{
  // check parameters
  if(grid_struct == NULL || pos == NULL) {
    return;
  }
  // ensure pos doesn't go out of bounds
  if (pos_get_x(pos) < 0 || pos_get_x(pos) >= grid_struct->nC
          || pos_get_y(pos) < 0 || pos_get_y(pos) >= grid_struct->nR) {
    return;
  }

  // if currently in passage-way:
  if(grid_struct->grid[pos_get_y(pos)][pos_get_x(pos)]->c == '#') {
    calculate_vision_passage(grid_struct, pos_get_x(pos), pos_get_y(pos));
    return;
  }
  // otherwise, look through all points in the grid
  for(int r = 0; r < grid_struct->nR; r++) {
    for(int c = 0; c < grid_struct->nC; c++) {
      // calculate whether they can see this point froom their current position
      if(calculate_vision(grid_struct, pos_get_x(pos), pos_get_y(pos), c, r)) {
        // if so, mark the point's visibility as true
        grid_struct->grid[r][c]->seen_before = true;
        grid_struct->grid[r][c]->visible_now = true;
      } else {
        grid_struct->grid[r][c]->visible_now = false;
      }
    }
  }
}

/**************** grid_delete ****************/
/* see grid.h for documentation */
void
grid_delete(grid_struct_t *grid_struct)
{
  if(grid_struct == NULL) {
    return;
  }
  // for each point in the grid
  for (int r = 0; r < grid_struct->nR; r++) {
    for (int c = 0; c < grid_struct->nC; c++) {
      point_delete(grid_struct->grid[r][c]); // free each *point_t
    }
    free(grid_struct->grid[r]); // free each **point_t
  }
  free(grid_struct->grid); // free the ***point_t
  free(grid_struct);
}

/**************** point_new ****************/
/* see grid.h for documentation */
point_t*
point_new(char c, bool seen, int gold_number)
{
  // allocate memory; error message on error
  point_t *point = count_malloc_assert(sizeof(point_t), "point_t");
  if (point == NULL) {
    return NULL;
  }
  point->c = c;
  point->seen_before = seen;
  point->visible_now = seen;
  point->gold_number = gold_number;
  return point;
}

/**************** point_delete ****************/
/* see grid.h for documentation */
void
point_delete(point_t *point)
{
  if(point == NULL) { // check parameters
    return;
  }
  free(point);
}

/**************** position_new ****************/
/* see grid.h for documentation */
position_t*
position_new(int x, int y)
{
  // allocate memory; error message on error
  position_t* pos = count_malloc_assert(sizeof(position_t), "position_t");
  if (pos == NULL) {
    return NULL;
  }
  pos->x = x;
  pos->y = y;
  return pos;
}

/**************** pos_update ****************/
/* see grid.h for documentation */
void
pos_update(position_t *pos, int x, int y)
{
  if(pos == NULL) { // check parameters
    return;
  }
  pos->x = x;
  pos->y = y;
}

/**************** pos_get_x ****************/
/* see grid.h for documentation */
int
pos_get_x(position_t *pos)
{
  if(pos == NULL) { // check parameters
    return -1;
  }
  return pos->x;
}

/**************** pos_get_y ****************/
/* see grid.h for documentation */
int
pos_get_y(position_t *pos)
{
  if(pos == NULL) { // check parameters
    return -1;
  }
  return pos->y;
}

/**************** position_delete ****************/
/* see grid.h for documentation */
void
position_delete(position_t *pos)
{
  if(pos == NULL) { // check parameters
    return;
  }
  free(pos);
}

/***********************************************************************
 * INTERNAL FUNCTIONS
 ***********************************************************************/

 /**************** calculate_vision_passage ****************/
 /* Helper method to calculate visibility when player is in a passageway ('#')
  *
  * Parameters:
  *   grid_struct   must be a grid* representing the grid to calculate
                    visbility on
  *     x           must be a valid int representing the x coordinate of the
                    current player location
  *     y           must be a valid int representing the y coordinate of the
                    current player location
  * Since this is a helper method called within our functions, we assume the
  * arguments provided in the function are valid.
  */
static void
calculate_vision_passage(grid_struct_t *grid_struct, int x, int y)
{
 // current spot the player is located at is always visible
 grid_struct->grid[y][x]->seen_before = true;
 grid_struct->grid[y][x]->visible_now = true;

 // check for visibility downward
 if(y+1 < grid_struct->nR) {
   if(grid_struct->grid[y+1][x]->c == '#' || grid_struct->grid[y+1][x]->c == '.') {
     grid_struct->grid[y+1][x]->seen_before = true;
     grid_struct->grid[y][x]->visible_now = true;
   }
 }
 // check for visibility upwards
 if(y-1 >= 0) {
   if(grid_struct->grid[y-1][x]->c == '#' || grid_struct->grid[y-1][x]->c == '.') {
     grid_struct->grid[y-1][x]->seen_before = true;
     grid_struct->grid[y][x]->visible_now = true;
   }
 }
 // check for visibility rightwards
 if(x+1 < grid_struct->nC) {
   if(grid_struct->grid[y][x+1]->c == '#' || grid_struct->grid[y][x+1]->c == '.') {
     grid_struct->grid[y][x+1]->seen_before = true;
     grid_struct->grid[y][x]->visible_now = true;
   }
 }
 // check for visibility leftwards
 if(x-1 >= 0 && grid_struct->grid[y][x-1]->c == '#') {
   if(grid_struct->grid[y][x-1]->c == '#' || grid_struct->grid[y][x-1]->c == '.') {
     grid_struct->grid[y][x-1]->seen_before = true;
     grid_struct->grid[y][x]->visible_now = true;
   }
 }
}

/**************** calculate_vision ****************/
/* Helper method to calculate visibility between two points
 *
 * Parameters:
 *   grid_struct   must be a grid* representing the grid to calculate
                   visbility on
 *     x1           must be a valid int representing the x coordinate of the
                   current player location
 *     y1           must be a valid int representing the y coordinate of the
                   current player location
 *     x2           must be a valid int representing the x coordinate of the
                    point to determine visibility towards
 *     y2           must be a valid int representing the y coordinate of the
                    point to determine visibility towards
 * Since this is a helper method called within our functions, we assume the
 * arguments provided in the function are valid.
 * We RETURN: TRUE if (x1,y1) to (x2,y2) is VISIBLE, otherwise we return FALSE
 */
static bool
calculate_vision(grid_struct_t *grid_struct, int x1, int y1, int x2, int y2)
{
  // 1. Comparing the SAME POINT -> always true
  if(x1 == x2 && y1 == y2) {
   return true;
  // 2. UNDEFINED SLOPE (vertical line)
  } else if(x1 == x2) {
   // go through each y in the line segment
   for(int y = 1; y <= abs(y2-y1) - 1; y++) {
     int curr_y;   // determining which one to start from (the smaller one)
     if(y1 > y2) {
       curr_y = y1 - y;
     } else {
       curr_y = y1 + y;
     }
     // if not a room spot, return false
     if(grid_struct->grid[curr_y][x1]->c != '.')  {
       return false;
     }
   }
 // 3. SLOPE OF ZERO (horizontal line)
 } else if (y1 == y2) {
   // go through each x in the line segment
   for(int x = 1; x <= abs(x2-x1) - 1; x++) {
     int curr_x;   // determining which one to start from (the smaller one)
     if(x1 > x2) {
       curr_x = x1 - x;
     } else {
       curr_x = x1 + x;
     }
     // if not a room spot, return false
     if(grid_struct->grid[y1][curr_x]->c != '.')  {
       return false;
     }
   }
 // 4. LINE SEGMENT
 } else {
   //// PART 1 - Going through each COLUMN (x) in the line segment
   double slope = (double) (y2 - y1) / (x2 - x1);  // calculate slope
   if(x1 < x2) {
     // For each x in the line segment:
     double curr_y = y1;
     for(int x = x1+1; x <= x2-1; x++) {
       curr_y += slope;  // find its respective y value
       // return false if non-room char encountered
       if(!calculate_helper_x(grid_struct, x, curr_y)) {
         return false;
       }
     }
   } else if (x1 > x2) {
     // For each x in the line segment:
     double curr_y = y1;
     for(int x = x1-1; x >= x2+1; x--) {
       curr_y = curr_y - slope; // find its respective y value
       // return false if non-room char encountered
       if(!calculate_helper_x(grid_struct, x, curr_y)) {
         return false;
       }
     }
   }

   //// PART 2 - Going through each ROW (y) in the line segment
   slope = (double) (x2 - x1) / (y2 - y1);   // calculate slope
   if(y1 < y2) {
     // For each y in the line segment:
     double curr_x = x1;
     for(int y = y1+1; y <= y2-1; y++) {
       curr_x += slope;  // find its respective x value
       // return false if non-room char encountered
       if(!calculate_helper_y(grid_struct, curr_x, y)) {
         return false;
       }
   }
 } else if (y1 > y2) {
     // For each y in the line segment:
     double curr_x = x1;
     for(int y = y1-1; y >= y2+1; y--) {
       curr_x = curr_x - slope;  // find its respective x value
       // return false if non-room char encountered
       if(!calculate_helper_y(grid_struct, curr_x, y)) {
         return false;
       }
     }
   }
 }
 // return true, only if it passes all the tests
 return true;
}

/**************** calculate_helper_x ****************/
/* Helper method used when looping through each x coordinate in a line
 * segment to calculate the visibility at a point (x,y)
 *
 * Parameters:
 *   grid_struct   must be a grid* representing the grid to calculate
                   visbility on
 *     x           must be a valid INT representing the x coordinate of the
                   point to calculate visibility with
 *     y           must be a valid DOUBLE representing the y coordinate of the
                   point to calculate visibility with
 * Since this is a helper method called within our functions, we assume the
 * arguments provided in the function are valid.
 * We RETURN: TRUE if (x,y) is VISIBLE, otherwise we return FALSE
 */
static bool
calculate_helper_x(grid_struct_t *grid_struct, int x, double y)
{
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
   // if gridpoint is not a 'room spot', return false
   if(grid_struct->grid[c][x]->c != '.')  {
     return false;
   }
  // if line segment passes between pairs of map gridpoints
  } else {
   // if both gridpoints are not a 'room spot', return false
   if(grid_struct->grid[c][x]->c != '.' && grid_struct->grid[f][x]->c != '.')  {
     return false;
   }
  }
  return true;
}

/**************** calculate_helper_y ****************/
/* Helper method used when looping through each y coordinate in a line
 * segment to calculate the visibility at a point (x,y)
 *
 * Parameters:
 *   grid_struct   must be a grid* representing the grid to calculate
                   visbility on
 *     x           must be a valid DOUBLE representing the x coordinate of the
                   point to calculate visibility with
 *     y           must be a valid INT representing the y coordinate of the
                   point to calculate visibility with
 * Since this is a helper method called within our functions, we assume the
 * arguments provided in the function are valid.
 * We RETURN: TRUE if (x,y) is VISIBLE, otherwise we return FALSE
 */
static bool
calculate_helper_y(grid_struct_t *grid_struct, double x, int y)
{
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
   // if gridpoint is not a 'room spot', return false
   if(grid_struct->grid[y][c]->c != '.')  {
     return false;
   }
  } else {
   // if both gridpoints are not a 'room spot', return false
   if(grid_struct->grid[y][c]->c != '.' && grid_struct->grid[y][f]->c != '.')  {
     return false;
   }
  }
  return true;
}
