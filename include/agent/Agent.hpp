#pragma once

#include "Player.hpp"

class Agent : public Player{
    public:
    Agent(std::string role);
    Action chooseMove() override;
};