#include "agent/Human.hpp"

#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

bool parseSquare(const std::string &text, int &row, int &col) {
    if (text.size() != 2)
        return false;
    col = std::tolower(static_cast<unsigned char>(text[0])) - 'a';
    row = text[1] - '1';
    return row >= 0 && row < 9 && col >= 0 && col < 9;
}

} // namespace

Move Human::chooseMove(const Position &, const GameHistory &) {
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
