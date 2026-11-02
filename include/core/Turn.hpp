#pragma once

#include <string>

class Turn {
private:
    std::string turn;

public:
    void setTurn(std::string givenTurn);
    void showState();
    std::string getString();
};