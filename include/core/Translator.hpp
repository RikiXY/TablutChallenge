#pragma once

#include "core/Rules.hpp"
#include <string>

Position parsePosition(const std::string &json);

std::string moveToJson(const Move &move, Side side);
std::string nameToJson(const std::string &name);

std::string convertToCoord(int row, int col);

bool parseSquare(const std::string &text, int &row, int &col);

std::string moveToText(const Move &move);
bool parseMove(const std::string &text, Move &move);
