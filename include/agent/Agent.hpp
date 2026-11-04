#pragma once

#include "Player.hpp"

struct NoLegalMoves {};

class Agent : public Player {
private:
    int timeoutSeconds;

public:
    explicit Agent(int timeoutSeconds);
    Move chooseMove(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point received) override;
};