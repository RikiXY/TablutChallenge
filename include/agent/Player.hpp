#pragma once

#include "core/Rules.hpp"
#include <chrono>

class Player {
public:
    virtual ~Player() = default; // Virtual destructor
    virtual Move chooseMove(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point startingTime) = 0; // Pure virtual method: must be implemented by derived classes
};