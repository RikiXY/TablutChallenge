#include "core/Board.hpp"
#include "core/Position.hpp"
#include <iostream>

void printBoard(const Position &position) {
    std::cout << "\n     a b c d e f g h i\n";
    std::cout << "   +-------------------+\n";

    for (int i = 0; i < 9; i++) {
        std::cout << " " << i + 1 << " | ";
        for (int j = 0; j < 9; j++) {
            const int square = i * 9 + j;
            const char *symbol = "?";
            switch (position.at(square)) {
            case Piece::Empty:
                if (square == THRONE)
                    symbol = "T";
                else if (isCamp(square))
                    symbol = "Δ";
                else
                    symbol = ".";
                break;
            case Piece::White:
                symbol = "W";
                break;
            case Piece::Black:
                symbol = "B";
                break;
            case Piece::King:
                symbol = "K";
                break;
            }
            std::cout << symbol << ' ';
        }
        std::cout << "| " << i + 1 << '\n';
    }
    std::cout << "   +-------------------+\n";
    std::cout << "     a b c d e f g h i\n\n";
    std::cout << "W = White | B = Black | K = King | T = Empty throne | Δ = Empty camp | . = Empty\n";
}