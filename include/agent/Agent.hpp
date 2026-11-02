#pragma once

#include "Player.hpp"
#include <vector>

class Agent : public Player {
private:
    std::vector<Move> generateAllMoves();

public:
    Agent(ServerConnector &gameConnection, std::string role);
    Action chooseMove() override;
};