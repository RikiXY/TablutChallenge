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

/**
 * @brief Tells which camp a cell belongs to
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return 0 top, 1 bottom, 2 left, 3 right, -1 if the cell isn't a camp
 */
int campOf(int row, int col);

/**
 * @brief Checks whether a cell is one of the 16 camp cells
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true if it's a camp cell
 */
bool isCamp(int row, int col);

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