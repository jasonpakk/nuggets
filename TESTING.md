# Nuggets: Testing Information

This document describes how we tested our modules and programs.

### server.c

We manually tested server.c by running client side commands and documenting the output as log files within our *log* subdirectory. \
For each item in the follow list, a separate log file is created with a relative path name of log/serverx.log where x is the respec\
ted number in the list i.e. the first item on the list is found in log/server1.log. All tests were run with the same seed (1) excep\
t for the one test for no seed provided. The server input is represented within the parentheses.

1. wrong number of arguments (./server 2> log/server1.log)
2. non-existent map (./server 2> log/server2.log maps/dummy.txt 1)
3. name length too long (abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz (length = 52))
4. Adding 27 players (./server 2> log/server4.log maps/main.txt 1)
5. Adding two players then removing a player
6. Adding two spectators
7. Map where number of free room spots is < min number of gold piles
8. Adding a player directly to a gold piece and immediately collecting gold (as no free room spots)
9. swapping two players
10. seeing a gold pile and then moving outside its visibility
11. collecting all the gold

Assumptions:
- The client only quits by pressing Q
- If there are no free room spots available and there

### lib/grid.c
