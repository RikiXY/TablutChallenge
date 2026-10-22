#pragma once

#include "core/GameState.hpp"
#include "connections/ServerConnector.hpp"

class Human{
    private:
    BoardMatrix boardMatrix;
    std::string role;

    public:
    void getGameState(GameState);
    void move(ServerConnector&, std::string, std::string);
};