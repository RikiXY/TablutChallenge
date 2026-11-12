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
 * @brief Converts a square to its name (21 -> d3)
 *
 * @param square The square
 * @return The name
 */
std::string convertToCoord(int square);

/**
 * @brief Reads a square name
 *
 * @param text The name
 * @param square Where to write the square
 * @return true if text is a square of the board
 */
bool parseSquare(const std::string &text, int &square);

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
