#pragma once

#include <string>
#include "core/GameState.hpp"
#include "core/Action.hpp"
#include "connections/ServerConnector.hpp"

class Player{
    protected:
    std::string role;
    Board board;
    bool myTurn = false;

    public:
    Player(std::string role);
    void getGameState(GameState gameState);  
    bool isMyTurn();
    virtual Action chooseMove() = 0;  // metodo virtuale puro che deve essere per forza definito dalle classi figlie
    void move(ServerConnector& gameConnection, Action action);
};