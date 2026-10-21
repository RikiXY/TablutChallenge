#pragma once

#include <string>
#include <array>

class Translator{
    public:
    std::array<std::array<int, 9>, 9> getBoard(std::string);  // input json → update current board
    std::string getTurn(std::string);  // input json → update current turn
};