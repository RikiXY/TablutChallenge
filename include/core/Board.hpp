#pragma once

#include <array>
#include <cstdint>

// What stands on a square. The throne and the camps are properties of the square, not pieces.
// uint8_t keeps the board at 81 bytes
enum class Piece : uint8_t { Empty,
                             White,
                             Black,
                             King };

using BoardMatrix = std::array<std::array<Piece, 9>, 9>;

struct Move {
    int fromRow, fromCol, toRow, toCol;
};

class Board {
private:
    BoardMatrix board{};

public:
    void setBoard(BoardMatrix givenBoard);
    void showState();
    void showNumericalState();
    BoardMatrix getBoardMatrix();
};
