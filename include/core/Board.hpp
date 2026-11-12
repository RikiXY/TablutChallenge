#pragma once

#include <cstdint>

// What stands on a square. The throne and the camps are properties of the square, not pieces.
// The server calls the empty throne "THRONE" in JSON, but it's just an empty square in "Piece" terms.
enum class Piece : uint8_t { Empty,
                             White,
                             Black,
                             King };

struct Move {
    uint8_t from, to;
    bool operator==(const Move &) const = default; // Automatically compare all members for equality
};

/**
 * @brief Makes a move from its squares
 *
 * @param from The square the piece starts from
 * @param to The square it moves to
 * @return The move
 */
constexpr Move makeMove(int from, int to) {
    return {static_cast<uint8_t>(from), static_cast<uint8_t>(to)};
}

constexpr int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}}; // left, right, up, down

// The camp of each square: 0 top, 1 bottom, 2 left, 3 right, -1 not a camp
constexpr int8_t CAMPS[81] = {
    -1,
    -1,
    -1,
    0,
    0,
    0,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    0,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    2,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    3,
    2,
    2,
    -1,
    -1,
    -1,
    -1,
    -1,
    3,
    3,
    2,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    3,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    1,
    1,
    1,
    -1,
    -1,
    -1,
};

/**
 * @brief Tells which camp a square belongs to
 *
 * @param square The square
 * @return 0 top, 1 bottom, 2 left, 3 right, -1 if the square isn't a camp
 */
constexpr int campOf(int square) {
    return CAMPS[square];
}

/**
 * @brief Checks whether a square is one of the 16 camp squares
 *
 * @param square The square
 * @return true if it's a camp square
 */
constexpr bool isCamp(int square) {
    return campOf(square) != -1;
}
