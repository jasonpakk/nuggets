/*
 * gridtest.c - unit test program for the Nuggets Project's grid module
 *
 * Code adapted from bagtest.c
 * Read the README or the TESTING.md file for more information.
 *
 * CS50, Team JEN, March 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"
#include "file.h"
#include "memory.h"
#include <math.h>

// file-local global variables
static int grid_unit_tested = 0;     // number of test cases run
static int grid_unit_failed = 0;     // number of test cases failed

// a macro for shorthand calls to expect()
#define EXPECT(cond) { unit_expect((cond), __LINE__); }

// Checks 'condition', increments grid_unit_tested, prints FAIL or PASS
void unit_expect(bool condition, int linenum)
{
  grid_unit_tested++;
  if (condition) {
    printf("PASS test %03d at line %d\n", grid_unit_tested, linenum);
  } else {
    printf("FAIL test %03d at line %d\n", grid_unit_tested, linenum);
    grid_unit_failed++;
  }
}

char c;     // char at the point
bool seen_before;   // has the player ever seen this point before
bool visible_now;   // is the point visible from a player's current position
int gold_number;  // amount of gold (gold piles)


/* **************************************** */
int main()
{
  printf("starting unit test for grid...\n");

  // initalize a new grid
  // small.txt represents a rectangle that has 5 rows and 14 columns
  // with 3 rows and 10 columns as empty spaces
  grid_struct_t *test_grid = grid_struct_new("../maps/small.txt");
  grid_load(test_grid, "../maps/small.txt", true);


  // compare the values returned by getter functions with expected values
  EXPECT(test_grid != NULL);
  EXPECT(grid_get_room_spot(test_grid) == 30);
  EXPECT(grid_get_nR(test_grid) == 5);
  EXPECT(grid_get_nC(test_grid) == 14);
  EXPECT(grid_get_point_c(test_grid, 6, 2) == '.');
  EXPECT(grid_get_point_gold(test_grid, 7, 3) == 0);

  // test setter function - character
  position_t *newCharPos = position_new(6, 2);

  char oldChar = grid_set_character(test_grid, '^', newCharPos);
  EXPECT(grid_get_point_c(test_grid, 6, 2) == '^');
  EXPECT(oldChar == '.');

  // test setter functions - gold number
  position_t *newGoldPos = position_new(7, 3);
  grid_set_gold(test_grid, 100, newGoldPos);
  EXPECT(grid_get_point_gold(test_grid, 7, 3) == 100);



  // testing error cases with getter functions
  EXPECT(grid_get_room_spot(NULL) == -1);
  EXPECT(grid_get_nR(NULL) == -1);
  EXPECT(grid_get_nC(NULL) == -1);
  EXPECT(grid_get_point_c(NULL, 0, 0) == '\0');
  EXPECT(grid_get_point_gold(NULL, 0, 0) == -1);

  // testing error cases with setter functions
  EXPECT(grid_set_character(NULL, '^', newCharPos) == '\0');
  EXPECT(grid_set_gold(NULL, 0, newGoldPos) == -1);


  // test grid_delete
  grid_delete(test_grid);

  printf("unit test complete\n");

  // print a summary
  if (grid_unit_failed > 0) {
    printf("FAILED %d of %d tests\n", grid_unit_failed, grid_unit_tested);
    return grid_unit_failed;
  } else {
    printf("PASSED all of %d tests\n", grid_unit_tested);
    return 0;
  }

  return 0;
}
