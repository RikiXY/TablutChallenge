#pragma once

#include "Player.hpp"

class Agent : public Player {
public:
    Agent(ServerConnector &gameConnection, std::string role);
    Action chooseMove() override;
};