#pragma once

#include "Player.hpp"

class Human : public Player{
    public:
    Human(std::string role);
    Action chooseMove() override;
};