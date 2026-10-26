#pragma once

#include <string>
#include "core/Board.hpp"
#include "core/Turn.hpp"

class Translator{
    private:
    Board transaltedBoard;
    Turn translatedTurn;

    public:
    Translator(std::string);
    Board getBoard();  // input json → update current board
    Turn getTurn();  // input json → update current turn
};