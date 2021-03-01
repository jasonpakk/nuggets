/*
 * server.c      Team Jen      March, 2021
 *
 * server.c    View README.md for functionality
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "support/message.h"
#include "support/log.h"


int game(int seed);
static bool handleMessage(void *arg, const addr_t from, const char *message);

typedef struct position {
  int x;
  int y;
} position_t;

typedef struct player {
  char *name;
  char *symbol;
  int gold_number;
  bool active;
  struct position pos;
} player_t;

typedef struct grid_struct {
  int nR;
  int nC;
  char** grid;
} grid_struct_t;

int
main(const int argc, const char *argv[])
{
  if (argc == 2 || argc == 3) {
    // no seed, generate one
    char* map_file = (char*)malloc(1+ strlen(argv[1])*sizeof(char));
    strcpy(map_file, argv[1]);
    if (argc == 2) {
      // generate a seed
        game(0);
    } else {
      //seed provided, scan it in and check it is positive
      int seed = atoi(argv[3]);
      if (seed <= 0) {
        fprintf(stderr, "the seed must be a positive integer.\n");
        return 2;
      }
      game(seed);
    }
  } else {
    // wrong number of arguments
    fprintf(stderr, "usage: ./server map.txt [seed]\n");
    return 1;
  }
}


int
game(int seed)
{
  if (seed == 0) {
    // generate a seed
  }
  addr_t other; // address of the other side of this communication (init below)

  log_init(stderr);
  // initialize the message module
  int ourPort = message_init(stderr);
  if (ourPort == 0) {
    return 2; // failure to initialize message module
  }
  printf("waiting on port %d for contact....\n", ourPort);
  other = message_noAddr(); // no correspondent yet
  bool ok = message_loop(&other, 0, NULL, NULL, handleMessage);
  message_done();
  return 0;
}


static bool
handleMessage(void *arg, const addr_t from, const char *message)
{
  addr_t *otherp = (addr_t *)arg;
  if (otherp == NULL) { // defensive
    log_v("handleMessage called with arg=NULL");
    return true;
  }

  // this sender becomes our correspondent, henceforth
  *otherp = from;
  printf("[%s@%05d]: %s\n",
         inet_ntoa(from.sin_addr), // IP address of the sender
         ntohs(from.sin_port),     // port number of the sender
         message);                 // message from the sender
  printf("this is message %s\n", message);
  fflush(stdout);
  return false;











}
