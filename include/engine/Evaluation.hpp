#pragma once

#include "core/Rules.hpp"

constexpr int WIN = 30000;
constexpr int WIN_BOUND = WIN - 1000;

int evaluate(const Position &position);
