#include "agent/Human.hpp"
#include "core/Translator.hpp"

#include <iostream>
#include <string>

Move Human::chooseMove(const Position &, const GameHistory &, std::chrono::steady_clock::time_point) {
    // ! Validate coordinate format only, allowing the human player to make illegal moves
    Move move{};
    while (true) {
        std::string from;
        std::string to;
        std::cout << "Enter starting position: ";
        std::cin >> from;
        std::cout << "Enter ending position: ";
        std::cin >> to;

        if (parseSquare(from, move.fromRow, move.fromCol) && parseSquare(to, move.toRow, move.toCol))
            return move;
        std::cout << "Not a square (use a1-i9), try again" << std::endl;
    }
}
