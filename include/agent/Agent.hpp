#pragma once

#include "Player.hpp"

class Agent : public Player {
public:
    Move chooseMove(const Position &position, const GameHistory &history) override;
};