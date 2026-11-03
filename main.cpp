#include "config/Config.hpp"
#include "connections/ServerConnector.hpp"
#include "core/Game.hpp"
#include <exception>
#include <iostream>

int main(int argc, char *argv[]) {
    try {
        const Config config = parseArguments(argc, argv);

        ServerConnector connection(config.ip, config.port);
        std::cout << "Connected" << std::endl;

        Game game;
        std::cout << "Joining game" << std::endl;
        game.start(connection, config);
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}