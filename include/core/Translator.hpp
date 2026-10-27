#pragma once

#include <string>
#include "core/Board.hpp"
#include "core/Turn.hpp"

class Translator{
    private:
    Board transaltedBoard;
    Turn translatedTurn;

    public:
    void update(std::string json);
    Board getBoard();  // input json → update current board
    Turn getTurn();  // input json → update current turn
};