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

void printBoard(const BoardMatrix &board);