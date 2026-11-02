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
The executable is `build/player`.

## Run
Start the Tablut server first, then:
```sh
./build/player <WHITE|BLACK> <timeout_s> <server_ip> -n <name> -m <auto|manual>
```
Example (one player per terminal):
```sh
./build/player WHITE 60 127.0.0.1 -n player1 -m auto
./build/player BLACK 60 127.0.0.1 -n player2 -m auto
```
WHITE connects on port 5800, BLACK on 5801.
