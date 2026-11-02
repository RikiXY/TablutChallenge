#pragma once

#include "Player.hpp"

class Human : public Player {
public:
    Human(ServerConnector &gameConnection, std::string role);
    Action chooseMove() override;
};