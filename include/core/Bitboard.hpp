#pragma once

#include "core/Board.hpp"
#include <array>
#include <cstdint>
#include <cstdlib>

// One bit per square, square = row * 9 + col
__extension__ typedef unsigned __int128 Bitboard;

/**
 * @brief Creates the bitboard of one square
 *
 * @param square The square
 * @return The bitboard with only that bit set
 */
constexpr Bitboard bit(int square) {
    return Bitboard(1) << square;
}

/**
 * @brief Counts the squares of a bitboard
 *
 * @param board The bitboard
 * @return The number of bits set
 */
inline int popcount(Bitboard board) {
    return __builtin_popcountll(static_cast<uint64_t>(board)) + __builtin_popcountll(static_cast<uint64_t>(board >> 64));
}

/**
 * @brief Finds the lowest square of a bitboard
 *
 * @param board The bitboard
 * @return The lowest square
 */
inline int lowestSquare(Bitboard board) {
    const auto low = static_cast<uint64_t>(board);
    return low ? __builtin_ctzll(low) : 64 + __builtin_ctzll(static_cast<uint64_t>(board >> 64));
}

/**
 * @brief Finds the highest square of a bitboard
 *
 * @param board The bitboard, not empty
 * @return The highest square
 */
inline int highestSquare(Bitboard board) {
    const auto high = static_cast<uint64_t>(board >> 64);
    return high ? 127 - __builtin_clzll(high) : 63 - __builtin_clzll(static_cast<uint64_t>(board));
}

/**
 * @brief Moves every square of a bitboard one step in a direction, dropping the ones that leave the board
 *
 * @param board The bitboard
 * @param direction The index in directions  (0 left, 1 right, 2 up, 3 down)
 * @return The shifted bitboard
 */
inline Bitboard shift(Bitboard board, int direction) {
    constexpr Bitboard all = (Bitboard(1) << 81) - 1;
    // 0x8040201008040201ull = a1..a8, 0x100 << 64 = a9
    constexpr Bitboard firstColumn = Bitboard(0x8040201008040201ull) | Bitboard(0x100) << 64;
    constexpr Bitboard lastColumn = firstColumn << 8;
    switch (direction) {
    case 0:
        return (board & ~firstColumn) >> 1;
    case 1:
        return (board & ~lastColumn) << 1;
    case 2:
        return board >> 9;
    default:
        return (board << 9) & all;
    }
}

constexpr int THRONE = 40; // e5, 9*4 + 4 = 40

// Precomputed squares and masks
struct BoardTables {
    Bitboard camps = 0;
    Bitboard campCenters = 0; // a5 e1 i5 e9
    Bitboard edges = 0;
    std::array<Bitboard, 4> campMasks{};                        // The cells of each camp
    std::array<int8_t, 81> campOf{};                            // The camp of each square, -1 if none
    std::array<Bitboard, 81> neighbours{};                      // The 4 squares next to each square
    std::array<std::array<int8_t, 4>, 81> neighbour{};          // The neighbour in each direction, -1 off the board
    std::array<std::array<std::array<int8_t, 8>, 4>, 81> ray{}; // The squares from each square to the edge in each direction, nearest first
    std::array<std::array<int8_t, 4>, 81> rayLength{};
    std::array<std::array<Bitboard, 4>, 81> rayMask{}; // The squares in a ray as a bitboard
};

/**
 * @brief Builds the tables at compile time
 *
 * @return The tables
 */
constexpr BoardTables makeTables() {
    BoardTables tables;
    for (int square = 0; square < 81; square++) {
        const int row = square / 9;
        const int col = square % 9;

        // Camps
        tables.campOf[square] = static_cast<int8_t>(campOf(square));
        if (isCamp(square)) {
            tables.camps |= bit(square);
            tables.campMasks[campOf(square)] |= bit(square);
        }
        if ((row == 4 && (col == 0 || col == 8)) || (col == 4 && (row == 0 || row == 8)))
            tables.campCenters |= bit(square);

        // Edges
        if (row == 0 || row == 8 || col == 0 || col == 8)
            tables.edges |= bit(square);

        // Neighbours and rays
        for (int direction = 0; direction < 4; direction++) {
            const int dRow = directions[direction][0];
            const int dCol = directions[direction][1];
            tables.neighbour[square][direction] = -1;
            int r = row + dRow;
            int c = col + dCol;
            if (r >= 0 && r < 9 && c >= 0 && c < 9) {
                tables.neighbour[square][direction] = static_cast<int8_t>(r * 9 + c);
                tables.neighbours[square] |= bit(r * 9 + c);
            }
            int length = 0;
            while (r >= 0 && r < 9 && c >= 0 && c < 9) {
                tables.ray[square][direction][length] = static_cast<int8_t>(r * 9 + c);
                tables.rayMask[square][direction] |= bit(r * 9 + c);
                r += dRow;
                c += dCol;
                length++;
            }
            tables.rayLength[square][direction] = static_cast<int8_t>(length);
        }
    }
    return tables;
}

inline constexpr BoardTables tables = makeTables();

/**
 * @brief Counts the free squares from a square in a direction, before the first blocker or the edge
 *
 * @param from The square
 * @param direction The direction
 * @param blockers The squares that stop the ray
 * @return How many squares in that direction are free
 */
inline int freeLength(int from, int direction, Bitboard blockers) {
    const Bitboard hit = tables.rayMask[from][direction] & blockers;
    if (!hit)
        return tables.rayLength[from][direction];
    const int first = direction == 1 || direction == 3 ? lowestSquare(hit) : highestSquare(hit);
    return std::abs(first - from) / (direction < 2 ? 1 : 9) - 1;
}

/**
 * @brief Finds the cells a piece can't stop on or pass through except the occupied ones
 *
 * @param from The square of the piece
 * @return The throne and the camps, except the piece's own camp
 */
inline Bitboard forbiddenFrom(int from) {
    const int camp = tables.campOf[from];
    return bit(THRONE) | (camp == -1 ? tables.camps : tables.camps & ~tables.campMasks[camp]);
}