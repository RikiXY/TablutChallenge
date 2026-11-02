#pragma once

#include "core/Rules.hpp"
#include <string>

Position parsePosition(const std::string &json);

std::string moveToJson(const Move &move, Side side);

std::string nameToJson(const std::string &name);
