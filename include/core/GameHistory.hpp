#pragma once

#include "core/Position.hpp"
#include <cstdint>
#include <vector>

struct GameHistory {
    int allowedRepetitions = 0;
    std::vector<uint64_t> sinceLastCapture;

    bool push(const Position &after, bool captured);
};