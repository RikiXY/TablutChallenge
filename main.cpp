// === CONVENZIONI DA RISPETTARE ===
// nomi cameCase, no snake_naming
// 1. i casting possono essere eseguiti o tramite reinterpret_cast<TYPE>(OBJ) (o static_cast<TYPE>(OBJ) ) o tramite TYPE(OBJ), niente (TYPE)OBJ

#include "config/Config.hpp"
#include "connections/ServerConnector.hpp"
#include "core/Game.hpp"
#include <cstdlib>
#include <ctime>
#include <exception>
#include <iostream>

// cmake --build build -j
// ./build/player WHITE 60 127.0.0.1 -n player1 -m auto
// ./build/player BLACK 60 127.0.0.1 -n player2 -m auto

int main(int argc, char *argv[]) {
    try {
        const Config config = parseArguments(argc, argv);

        ServerConnector connection(config.ip, config.port);
        std::cout << "Connected" << std::endl;

        srand(time(nullptr));

        Game game;
        std::cout << "Joining game" << std::endl;
        game.start(connection, config);
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}