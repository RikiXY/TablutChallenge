#pragma once

#include "core/Rules.hpp"
#include <chrono>

class Player {
public:
    virtual ~Player() = default;
    // metodo virtuale puro che deve essere per forza definito dalle classi figlie
    virtual Move chooseMove(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point startingTime) = 0;
};