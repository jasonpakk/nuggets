# Nuggets
### Team Name: Team JEN
#### Team Members: Nina Paripovic, Egemen Sahin, Jason Pak

This repository contains the code for the CS50 "Nuggets" game, in which players explore a set of rooms and passageways in search of gold nuggets.
The rooms and passages are defined by a *map* loaded by the server at the start of the game.
The gold nuggets are randomly distributed in *piles* within the rooms.
Up to 26 players, and one spectator, may play a given game.
Each player is randomly dropped into a room when joining the game.
Players move about, collecting nuggets when they move onto a pile.
When all gold nuggets are collected, the game ends and a summary is printed.

To build, run `make all`.

To clean up, run `make clean`.

## Requirements spec

See the [Requirements Spec](REQUIREMENTS.md) for details about the game and its network protocol.

## Design spec

See the [Design Spec](DESIGN.md) for details about the design plan our group took to create the nuggets game.

## Implementation spec

See the [Implementation Spec](IMPLEMENTATION.md) for details about how our group implemented the requirements of the Nuggets game into our program.

## Server
To run the server for the NUGGETS Project, call:
	./server map.txt [seed]

## Subdirectories

We created a new subdirectory named [lib](lib/README.md) which stores useful modules that we used for our implementation of the Nuggets game.
`grid.h` and `server_player` are modules that our group wrote for this project. The other modules were provided in previous CS 50 Labs.

The [support](support/README.md) subdirectory is a library that contains the provided modules for the Nuggets Project.

The [maps](maps/README.md) subdirectory contains available for testing. We contributed a new map for testing, and the map is titled `TEAM_JEN_MAP.txt`

The [images](images/README.md) subdirectory contains images and files used for the documentation of the Nuggets Project.

## Testing
See the [Testing Document](TESTING.md) for details about how our group tested our modules and programs.
