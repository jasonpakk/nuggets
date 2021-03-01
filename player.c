/*
 * player.c      Team Jen      March, 2021
 *
 * player.c    View README.md for functionality
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "support/message.h"
#include "support/log.h"

static bool handleMessage(void *arg, const addr_t from, const char *message);
static bool handleInput(void *arg);
static bool readline(char *buf, const int len);

int
main(const int argc, const char *argv[])
{
  const char *program = argv[0];
  if (argc != 3) {
    fprintf(stderr, "usage: %s hostname port\n", program);
    return 3; // bad commandline
  } else {
    addr_t other; // address of the other side of this communication (init below)
    other = message_noAddr(); // no correspondent yet
    const char *otherHost = argv[1];
    const char *otherPort = argv[2];
    if (message_setAddr(otherHost, otherPort, &other)) {
      // initiate communication
      message_send(other, "hello!");
      printf("Write a message....\n");
    } else {
      fprintf(stderr, "can't form address from %s %s\n", otherHost, otherPort);
      return 4; // bad hostname/port
    }
    message_loop(&other, 0, NULL, handleInput, handleMessage);
    message_done();
    return 0;
  }

}

static bool
handleInput(void *arg)
{
  addr_t *otherp = arg;
  if (otherp == NULL) { // defensive
    log_v("handleInput called with arg=NULL");
    return true;
  }

  // allocate a buffer into which we can read a line of input
  // (it can't be any bigger than a message)
  char line[message_MaxBytes];

  // read a line from stdin
  if (!readline(line, message_MaxBytes)) {
    return true; // EOF
  }

  // try to send the line to our correspondent
  if (!message_isAddr(*otherp)) {
    log_v("handleInput called without a correspondent.");
    printf("You have no correspondent.\n");
    fflush(stdout);
    return false;
  } else {
    message_send(*otherp, line);
    return false;
  }
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
  fflush(stdout);
  return false;
}

static bool
readline(char *buf, const int len)
{
  int pos = 0;          // where in the buffer do we place next char?
  char c = '\0';

  // Fill the buffer from stdin until buf is full, until EOF, or until newline
  while ( (c = getchar()) != EOF  &&  c != '\n' &&  pos < len-1 ) {
    // add char to the buffer
    buf[pos++] = c;
  }
  // terminate buffer
  buf[pos] = '\0';

  if (c == '\n') {	             // end of line discovered
    return true;
  } else if (pos == 0 && c == EOF) { // EOF without input
    return false;
  } else {		             // buffer filled, but no newline
    // skip characters until end of file or newline is finally reached.
    while ( !feof(stdin) && getchar() != '\n' ) {
      ; // discard the rest of characters on input line
    }
    return true;
  }
}
