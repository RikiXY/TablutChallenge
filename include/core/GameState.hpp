#pragma once

#include "Board.hpp"
#include "Turn.hpp"

class GameState{
    private:
    Board currentBoard;
    Turn currentTurn;
    bool finished = false;

    public:
    void update(std::string);
    Board getBoard();
    Turn getTurn();
    bool isOver();
    std::string winner();
};