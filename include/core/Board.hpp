#pragma once

#include <array>
#include <string>

using BoardMatrix = std::array<std::array<uint8_t, 9>, 9>;

class Board{
    private:
    BoardMatrix board {};  // con {} inizializzo tutto a 0
    
    public:
    void updateState(std::string);
    void showState();
    BoardMatrix getBoardMatrix();
};
