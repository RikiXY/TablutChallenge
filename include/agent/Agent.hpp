#pragma once

#include "Player.hpp"

class Agent : public Player {
private:
    int timeoutSeconds;

public:
    explicit Agent(int timeoutSeconds);
    Move chooseMove(const Position &position, const GameHistory &history) override;
};