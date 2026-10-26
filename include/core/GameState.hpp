#pragma once

#include "connections/ServerConnector.hpp"
#include "Board.hpp"
#include "Turn.hpp"

class GameState{
    private:
    ServerConnector& gameConnection;
    Board currentBoard;
    Turn currentTurn;
    bool finished = false;

    public:
    GameState(ServerConnector& gameConnection);
    void update();
    Board getBoard();
    Turn getTurn();
    bool isOver();
    std::string winner();
};