#pragma once

#include <array>
#include <string>

using BoardMatrix = std::array<std::array<int, 9>, 9>;

class Board{
    private:
    BoardMatrix currentBoard {};  // con {} inizializzo tutto a 0
    
    public:
    void updateState(std::string rawJson);
    void showState();
};
