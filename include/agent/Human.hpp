#pragma once

#include "Player.hpp"

class Human : public Player {
public:
    Move chooseMove(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point received) override;
};