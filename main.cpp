// === CONVENZIONI DA RISPETTARE ===
// nomi cameCase, no snake_naming
// 1. i casting possono essere eseguiti o tramite reinterpret_cast<TYPE>(OBJ) (o static_cast<TYPE>(OBJ) ) o tramite TYPE(OBJ), niente (TYPE)OBJ

#include "config/Config.hpp"
#include "connections/ServerConnector.hpp"
#include "core/Game.hpp"
#include "core/Translator.hpp"
#include "engine/Search.hpp"
#include <algorithm>
#include <chrono>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

// cmake --build build -j
// ./build/player WHITE 60 127.0.0.1 -n player1 -m auto
// ./build/player BLACK 60 127.0.0.1 -n player2 -m auto
// ./build/player think 100 e3d3 a4b4

namespace {

void think(int argc, char *argv[]) {
    // Apply the previous moves
    Position position = Position::start();
    GameHistory history;
    for (int i = 3; i < argc; i++) {
        Move move{};
        if (!parseMove(argv[i], move))
            throw std::invalid_argument("Not a move: " + std::string(argv[i]));

        const std::vector<Move> legal = position.calculateLegalMoves();
        if (position.outcome != Outcome::Ongoing || std::find(legal.begin(), legal.end(), move) == legal.end())
            throw std::invalid_argument("Illegal move: " + std::string(argv[i]));

        const bool captured = position.apply(move) > 0;
        history.push(position, captured);
    }
    if (position.outcome != Outcome::Ongoing || position.calculateLegalMoves().empty())
        throw std::invalid_argument("The game is already over");

    // Get the best move with the given time limit
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(std::stoi(argv[2]));
    const SearchResult result = search(position, history, deadline);
    std::cout << "bestmove " << moveToText(result.bestMove) << " depth " << result.depth
              << " score " << result.score << " nodes " << result.nodes << std::endl;
}

} // namespace

int main(int argc, char *argv[]) {
    try {
        if (argc >= 3 && std::string_view(argv[1]) == "think") {
            think(argc, argv);
            return 0;
        }

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