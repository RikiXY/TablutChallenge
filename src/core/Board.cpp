#include "core/Board.hpp"
#include <iostream>

bool isEdge(int row, int col) {
    return row == 0 || row == 8 || col == 0 || col == 8;
}

int campOf(int row, int col) {
    if ((row == 0 || row == 8) && col >= 3 && col <= 5)
        return row == 0 ? 0 : 1;
    if ((col == 0 || col == 8) && row >= 3 && row <= 5)
        return col == 0 ? 2 : 3;
    if (row == 1 && col == 4)
        return 0;
    if (row == 7 && col == 4)
        return 1;
    if (row == 4 && col == 1)
        return 2;
    if (row == 4 && col == 7)
        return 3;
    return -1;
}

bool isCamp(int row, int col) {
    return campOf(row, col) != -1;
}

bool isCampCenter(int row, int col) {
    return (row == 0 && col == 4) || (row == 8 && col == 4) || (row == 4 && col == 0) || (row == 4 && col == 8);
}

void printBoard(const BoardMatrix &board) {
    std::cout << "\n     a b c d e f g h i\n";
    std::cout << "   +-------------------+\n";

    for (int i = 0; i < 9; i++) {
        std::cout << " " << i + 1 << " | ";
        for (int j = 0; j < 9; j++) {
            const char *symbol = "?";
            switch (board[i][j]) {
            case Piece::Empty:
                if (i == 4 && j == 4)
                    symbol = "T";
                else if (isCamp(i, j))
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