# Nuggets: Testing Information

This document describes how we tested our modules and programs.


(all with seed 1 except test for no seed)
1. wrong number of arguments
2. non-exsistient map (ADD DEFENSIVE)
3. name length too long (sidgiuhgshgofihgvoigjsfogijofsigjiosgjosgjofgjejeofjigbhgfdfgbgffgb)
4. Adding 27 players
5. Removing a player
6. Adding two spectators
7. Map where number of free room spots is < min number of gold piles
8. Map where number of free room spots is < max number of gold piles
9. Adding a player to gold saturated map and immediately collecting gold
10. seeing a gold pile and then moving outside its visibility
11. seeing a part of grid and moving outside its visibility
12. collecting all the gold
13. swapping two players
14. moving immediately between two passageways (?)


Assumption: the client only quits by pressing Q
