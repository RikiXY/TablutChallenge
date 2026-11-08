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

/**
 * @brief Checks whether a cell is inside the board
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true if both arguments are valid
 */
constexpr bool isOnBoard(int row, int col) {
    return row >= 0 && row < 9 && col >= 0 && col < 9;
}

/**
 * @brief Checks whether a cell is the throne (e5)
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true if it's the throne
 */
constexpr bool isThrone(int row, int col) {
    return row == 4 && col == 4;
}

/**
 * @brief Checks whether a cell is on the edge of the board, where the king escapes
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true if it's on the first or last row or column
 */
bool isEdge(int row, int col);

// The camp of each cell: 0 top, 1 bottom, 2 left, 3 right, -1 not a camp
constexpr int8_t CAMPS[9][9] = {
    {-1, -1, -1, 0, 0, 0, -1, -1, -1},
    {-1, -1, -1, -1, 0, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, -1, -1, -1, -1, -1, -1, -1, 3},
    {2, 2, -1, -1, -1, -1, -1, 3, 3},
    {2, -1, -1, -1, -1, -1, -1, -1, 3},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, 1, -1, -1, -1, -1},
    {-1, -1, -1, 1, 1, 1, -1, -1, -1},
};

/**
 * @brief Tells which camp a cell belongs to
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return 0 top, 1 bottom, 2 left, 3 right, -1 if the cell isn't a camp
 */
constexpr int campOf(int row, int col) {
    return CAMPS[row][col];
}

/**
 * @brief Checks whether a cell is one of the 16 camp cells
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true if it's a camp cell
 */
constexpr bool isCamp(int row, int col) {
    return campOf(row, col) != -1;
}

/**
 * @brief Checks whether a cell is the middle cell of a camp (e1, e9, a5, i5)
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true if it's a camp center
 */
bool isCampCenter(int row, int col);

/**
 * @brief Prints the board on the terminal
 *
 * @param board The board to print
 */
void printBoard(const BoardMatrix &board);