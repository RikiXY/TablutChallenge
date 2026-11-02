// === CONVENZIONI DA RISPETTARE ===
// nomi cameCase, no snake_naming
// 1. i casting possono essere eseguiti o tramite reinterpret_cast<TYPE>(OBJ) (o static_cast<TYPE>(OBJ) ) o tramite TYPE(OBJ), niente (TYPE)OBJ

#include <iostream>
#include "core/Parser.hpp"
#include "connections/ServerConnector.hpp"
#include <ctime>
#include "core/Game.hpp"

// #define NAME "CustomFish"

// cmake --build build -j
// ./build/player WHITE 60 127.0.0.1 -n player1 -m auto
// ./build/player BLACK 60 127.0.0.1 -n player2 -m auto

int main(int argc, char *argv[]){

    Parser parser(argc, argv);

    ServerConnector connection(parser.ip, parser.port);
    connection.connectToServer();
    std::cout<<"Connected"<<std::endl;

    srand(time(nullptr));

    Game game;
    std::cout<<"Joining game"<<std::endl;
    game.start(connection, parser.name, parser.role, parser.playMode);  // Join game on <connection> with <name> id
    
    connection.endConnection();

    return 0;
}