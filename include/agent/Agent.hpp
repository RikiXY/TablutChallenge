#pragma once

#include "Player.hpp"
#include <vector>

class Agent : public Player{
    private:
    std::vector<std::array<std::string, 2>> generateTree();

    public:
    Agent(ServerConnector& gameConnection, std::string role);
    Action chooseMove() override;
};