#pragma once

#include "Player.hpp"

class Human : public Player {
public:
    Move chooseMove(const Position &position, const GameHistory &history) override;
};