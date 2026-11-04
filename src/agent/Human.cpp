#include "agent/Human.hpp"
#include "core/Translator.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

Move Human::chooseMove(const Position &, const GameHistory &, std::chrono::steady_clock::time_point) {
    // ! non è presente validazione delle mosse perché il giocatore umano deve poter sbagliare
    Move move{};
    while (true) {
        std::string from;
        std::string to;
        std::cout << "Enter starting position: ";
        std::cin >> from;
        std::cout << "Enter ending position: ";
        std::cin >> to;
        if (!std::cin)
            throw std::runtime_error("Input closed");

        if (parseSquare(from, move.fromRow, move.fromCol) && parseSquare(to, move.toRow, move.toCol))
            return move;
        std::cout << "Not a square (use a1-i9), try again" << std::endl;
    }
}
