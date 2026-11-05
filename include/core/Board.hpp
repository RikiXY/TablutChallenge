#pragma once

#include <array>
#include <cstdint>

// What stands on a square. The throne and the camps are properties of the square, not pieces.
// The server calls the empty throne "THRONE" in JSON, but it's just an empty square in "Piece" terms.
// uint8_t keeps the board at 81 bytes
enum class Piece : uint8_t { Empty,
                             White,
                             Black,
                             King };

using BoardMatrix = std::array<std::array<Piece, 9>, 9>;

struct Move {
    int fromRow, fromCol, toRow, toCol;
    bool operator==(const Move &) const = default; // Automatically compare all members for equality
};

constexpr int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}}; // left, right, up, down

constexpr bool isOnBoard(int row, int col) {
    return row >= 0 && row < 9 && col >= 0 && col < 9;
}

constexpr bool isThrone(int row, int col) {
    return row == 4 && col == 4;
}

bool isEdge(int row, int col);
int campOf(int row, int col);
bool isCamp(int row, int col);
bool isCampCenter(int row, int col);

void printBoard(const BoardMatrix &board);