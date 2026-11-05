#pragma once

#include "core/Position.hpp"
#include <cstdint>
#include <vector>

struct GameHistory {
    int allowedRepetitions = 0;
    std::vector<uint64_t> sinceLastCapture;

    /**
     * @brief Adds a position to the history and checks for a draw by repetition
     *
     * @param after The position after the move
     * @param captured Whether the move captured a pawn (clears the history)
     * @return true if the position was already seen more than allowedRepetitions times
     */
    bool push(const Position &after, bool captured);
};