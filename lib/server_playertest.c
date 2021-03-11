/*
 * server_playertest.c - unit test program for the Nuggets Project's server_player module
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
#include "memory.h"
#include "message.h"
#include "server_player.h"

// file-local global variables
static int unit_tested = 0;     // number of test cases run
static int unit_failed = 0;     // number of test cases failed

// a macro for shorthand calls to expect()
#define EXPECT(cond) { unit_expect((cond), __LINE__); }

// Checks 'condition', increments unit_tested, prints FAIL or PASS
void unit_expect(bool condition, int linenum)
{
  unit_tested++;
  if (condition) {
    printf("PASS test %03d at line %d\n", unit_tested, linenum);
  } else {
    printf("FAIL test %03d at line %d\n", unit_tested, linenum);
    unit_failed++;
  }
}

/* **************************************** */
int main()
{
  printf("starting unit test for server_player...\n");

  // initalize a new server_player
  addr_t add = message_noAddr();
  position_t *pos = position_new(1, 1);
  server_player_t *player = server_player_new(add, "testname", 'a', true, pos);

  // compare the values returned by getter functions with expected values
  EXPECT(player != NULL);
  EXPECT(strcmp(server_player_getName(player),"testname") == 0);
  EXPECT(server_player_getSymbol(player) == 'a');
  EXPECT(message_eqAddr(server_player_getAddress(player), message_noAddr()) == true);
  EXPECT(server_player_getGoldNumber(player) == 0);
  EXPECT(server_player_getGoldPickedUp(player) == 0);
  EXPECT(server_player_getActive(player) == true);
  EXPECT(server_player_getPos(player) == pos);
  EXPECT(pos_get_x(server_player_getPos(player)) == 1);
  EXPECT(pos_get_y(server_player_getPos(player)) == 1);
  EXPECT(server_player_getGrid(player) == NULL);
  EXPECT(server_player_getInPassage(player) == false);

  // test setter function - name
  char* newname = malloc(10);
  strcpy(newname, "newname");
  free(server_player_getName(player));
  server_player_setName(player, newname);
  EXPECT(server_player_getName(player) == newname);
  EXPECT(strcmp(server_player_getName(player),"newname") == 0);

  // test setter function - symbol
  server_player_setSymbol(player, 'z');
  EXPECT(server_player_getSymbol(player) == 'z');

  // test setter functions - address
  message_setAddr("localhost", "12345", &add);
  server_player_setAddress(player, add);
  EXPECT(message_eqAddr(server_player_getAddress(player), add) == true);

  // test setter functions - gold number
  server_player_setGoldNumber(player, 10);
  EXPECT(server_player_getGoldNumber(player) == 10);

  // test setter functions - gold picked up
  server_player_setGoldPickedUp(player, 5);
  EXPECT(server_player_getGoldPickedUp(player) == 5);

  // test setter functions - active
  server_player_setActive(player, false);
  EXPECT(server_player_getActive(player) == false);

  // test setter functions - pos
  position_delete(server_player_getPos(player));
  position_t *new_pos = position_new(3, 3);
  server_player_setPos(player, new_pos);
  EXPECT(server_player_getPos(player) == new_pos);
  EXPECT(pos_get_x(server_player_getPos(player)) == 3);
  EXPECT(pos_get_y(server_player_getPos(player)) == 3);

  // test setter functions - grid
  // (Note: must have a valid main.txt map file in the maps directory)
  grid_struct_t *new_grid = grid_struct_new("../maps/main.txt");
  grid_load(new_grid, "../maps/main.txt", true);
  server_player_setGrid(player, new_grid);
  EXPECT(server_player_getGrid(player) == new_grid);

  // test setter functions - in passage
  server_player_setInPassage(player, true);
  EXPECT(server_player_getInPassage(player) == true);

  // test player_delete
  server_player_delete(player);

  // initalize a new server_player as a spectator
  addr_t spect_addr = message_noAddr();
  server_player_t *spectator = server_player_new(spect_addr, "SPECTATOR", '!', false, NULL);

  // compare the values returned by getter functions with expected values
  EXPECT(spectator != NULL);
  EXPECT(strcmp(server_player_getName(spectator),"SPECTATOR") == 0);
  EXPECT(server_player_getSymbol(spectator) == '!');
  EXPECT(message_eqAddr(server_player_getAddress(spectator), message_noAddr()) == true);
  EXPECT(server_player_getGoldNumber(spectator) == 0);
  EXPECT(server_player_getGoldPickedUp(spectator) == 0);
  EXPECT(server_player_getActive(spectator) == false);
  EXPECT(server_player_getPos(spectator) == NULL);
  EXPECT(server_player_getGrid(spectator) == NULL);
  EXPECT(server_player_getInPassage(spectator) == false);

  // test server_spectator_delete
  server_spectator_delete(spectator);

  // testing error cases with getter functions
  EXPECT(server_player_getName(NULL) == NULL);
  EXPECT(server_player_getSymbol(NULL) == '\0');
  EXPECT(message_eqAddr(server_player_getAddress(NULL), message_noAddr()) == true);
  EXPECT(server_player_getGoldNumber(NULL) == 0);
  EXPECT(server_player_getGoldPickedUp(NULL) == 0);
  EXPECT(server_player_getActive(NULL) == false);
  EXPECT(server_player_getPos(NULL) == NULL);
  EXPECT(server_player_getGrid(NULL) == NULL);
  EXPECT(server_player_getInPassage(NULL) == false);

  // testing error cases with setter functions
  EXPECT(server_player_setName(NULL, NULL) == false);
  EXPECT(server_player_setSymbol(NULL, 'c') == false);
  EXPECT(server_player_setAddress(NULL, message_noAddr()) == false);
  EXPECT(server_player_setGoldNumber(NULL, 0) == false);
  EXPECT(server_player_setGoldPickedUp(NULL, 0) == false);
  EXPECT(server_player_setActive(NULL, false) == false);
  EXPECT(server_player_setPos(NULL, NULL) == false);
  EXPECT(server_player_setGrid(NULL, NULL) == false);
  EXPECT(server_player_setInPassage(NULL, false) == false);

  printf("unit test complete\n");

  // print a summary
  if (unit_failed > 0) {
    printf("FAILED %d of %d tests\n", unit_failed, unit_tested);
    return unit_failed;
  } else {
    printf("PASSED all of %d tests\n", unit_tested);
    return 0;
  }

  return 0;
}
