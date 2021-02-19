# Nuggets

This repository contains the code for the CS50 "Nuggets" game, in which players explore a set of rooms and passageways in search of gold nuggets.
The rooms and passages are defined by a *map* loaded by the server at the start of the game.
The gold nuggets are randomly distributed in *piles* within the rooms.
Up to 26 players, and one spectator, may play a given game.
Each player is randomly dropped into a room when joining the game.
Players move about, collecting nuggets when they move onto a pile.
When all gold nuggets are collected, the game ends and a summary is printed.

## Requirements spec

See the [Requirements Spec](REQUIREMENTS.md) for details about the game and its network protocol.

## Materials provided

See the [support library](support/README.md) for some useful modules.

See the [maps](maps/README.md) for some draft maps.

## Updating your starter kit

To update your repo from the starter kit, *commit and push* your work, then

    # do this once:
    git remote add starter git@github.com:cs50spring2019/nuggets.git
    # do this every time you want to update:
    git pull starter master

much like in these [instructions](https://www.cs.dartmouth.edu/~cs50/Labs/updates.html).
