#pragma once

#include "core/GameHistory.hpp"
#include "core/Position.hpp"
#include <chrono>

class Player {
public:
    virtual ~Player() = default; // Virtual destructor

    /**
     * @brief Chooses the move to play
     *
     * @param position The current position
     * @param history The game history (for the repetitions)
     * @param startingTime When the state was received, used by the move timer
     * @return The move to play
     */
    virtual Move chooseMove(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point startingTime) = 0; // Pure virtual method: must be implemented by derived classes
};