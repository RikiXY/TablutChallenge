#include "core/Board.hpp"
#include <iostream>

void Board::setBoard(BoardMatrix givenBoard) {
    board = givenBoard;
}

void Board::showNumericalState() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            std::cout << static_cast<int>(board[i][j]);
        }
        std::cout << std::endl;
    }
}

void Board::showState() {
    std::cout << "\n     a b c d e f g h i\n";
    std::cout << "   +-------------------+\n";

    for (int i = 0; i < 9; i++) {
        std::cout << " " << i + 1 << " | ";
        for (int j = 0; j < 9; j++) {
            char symbol = '?';
            switch (board[i][j]) {
            case Piece::Empty:
                symbol = (i == 4 && j == 4) ? 'T' : '.';
                break;
            case Piece::White:
                symbol = 'W';
                break;
            case Piece::Black:
                symbol = 'B';
                break;
            case Piece::King:
                symbol = 'K';
                break;
            }
            std::cout << symbol << ' ';
        }
        std::cout << "| " << i + 1 << '\n';
    }
    std::cout << "   +-------------------+\n";
    std::cout << "     a b c d e f g h i\n\n";
    std::cout << "W = White | B = Black | K = King | T = Empty throne | . = Empty\n";
}

BoardMatrix Board::getBoardMatrix() {
    return board;
}
