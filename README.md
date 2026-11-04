# TablutChallenge
C++ project for the UniBO Tablut Challenge, including game logic, server communication, search-based AI, and neural evaluation.

## Requirements
- CMake >= 3.16
- A C++20 compiler

## Build
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```
The executables are `build/player` (tournament player) and `build/arena` (self-play testing).

## Run
Start the Tablut server first, then:
```sh
./build/player <WHITE|BLACK> <timeout_s> <server_ip> [-n <name>] [-m <auto|manual>]
```
Example (one player per terminal):
```sh
./build/player WHITE 60 127.0.0.1 -n player1 -m auto
./build/player BLACK 60 127.0.0.1 -n player2 -m auto
```
WHITE connects on port 5800, BLACK on 5801. Defaults: name `CustomFish`, mode `auto`.

## Arena
Plays two engines against each other using random openings, each opening with the colors swapped,
and stops when it's clear whether A is stronger than B.
To test a change:
```sh
cp build/player build/player-base # version B, before the change
# Then edit and build thew new version of the player, which will be version A
./build/arena -a build/player -b build/player-base -ms 100 -c 8 -games logs/games.txt
```
Options: `-ms` time per move, `-c` games at a time, `-pairs` maximum pairs (1000), `-plies` random opening moves (6),
`-seed` (1), `-elo0`/`-elo1` the SPRT hypotheses (0 and 10 Elo), `-games` appends every game as `WHITE_SCORE FAILED MOVES...`.
An engine can be any command that answers `<command> think <ms> <moves...>` with `bestmove e3d3 ...`.