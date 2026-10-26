#pragma once

#include "Player.hpp"
#include <vector>

class Agent : public Player{
    private:
    std::vector<std::array<std::string, 2>> generateMoves(BoardMatrix initialState, uint8_t X, uint8_t Y);
    std::vector<std::array<std::string, 2>> generateTree();

    public:
    Agent(std::string role);
    Action chooseMove() override;
};