#pragma once

#include "core/Position.hpp"
#include <string>

/**
 * @brief Converts the state sent by the server into a Position
 *
 * @param json The state sent by the server
 * @return The position
 */
Position parsePosition(const std::string &json);

/**
 * @brief Writes a move in the JSON format
 *
 * @param move The move
 * @param side The side making the move
 * @return The JSON (eg. {"from":"e3","to":"e4","turn":"WHITE"})
 */
std::string moveToJson(const Move &move, Side side);

/**
 * @brief Writes the player name as a JSON string
 *
 * @param name The player name
 * @return The name as a JSON string
 */
std::string nameToJson(const std::string &name);

/**
 * @brief Converts a cell to its name (2,3 -> d3)
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return The name
 */
std::string convertToCoord(int row, int col);

/**
 * @brief Reads a cell name
 *
 * @param text The name
 * @param row Where to write the row
 * @param col Where to write the column
 * @return true if text is a cell of the board
 */
bool parseSquare(const std::string &text, int &row, int &col);

/**
 * @brief Writes a move as text (eg. d6d7)
 *
 * @param move The move
 * @return The move
 */
std::string moveToText(const Move &move);

/**
 * @brief Reads a move written as text
 *
 * @param text The move
 * @param move Where to write the move
 * @return true if the format is right
 */
bool parseMove(const std::string &text, Move &move);
