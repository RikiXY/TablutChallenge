#pragma once

#include <string>
#include <array>
#include <vector>

using BoardMatrix = std::array<std::array<uint8_t, 9>, 9>;

class Translator{
    private:
    BoardMatrix board;
    std::string turn;

    public:
    Translator(std::string);
    BoardMatrix getBoard();  // input json → update current board
    std::string getTurn();  // input json → update current turn
};