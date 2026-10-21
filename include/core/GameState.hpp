#pragma once

#include "Board.hpp"
#include "Turn.hpp"

class GameState{
    private:
    Board currentBoard;
    Turn currentTurn;

    public:
    GameState(std::string);
    Board getBoard();
    Turn getTurn();
};