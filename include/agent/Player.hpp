#pragma once

#include "core/Rules.hpp"

class Player {
public:
    virtual ~Player() = default; // Virtual destructor
    virtual Move chooseMove(const Position &position, const GameHistory &history) = 0; // Pure virtual method: must be implemented by derived classes
};