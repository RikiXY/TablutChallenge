#pragma once

#include "connections/ServerConnector.hpp"
#include "core/Action.hpp"
#include "core/GameState.hpp"
#include <string>

class Player {
protected:
    ServerConnector &gameConnection;
    std::string role;
    Board board;
    bool myTurn = false;

public:
    Player(ServerConnector &gameConnection, std::string role);
    virtual ~Player() = default;
    void setGameState(GameState gameState);
    bool isMyTurn();
    virtual Action chooseMove() = 0; // metodo virtuale puro che deve essere per forza definito dalle classi figlie
    void move(Action action);
};