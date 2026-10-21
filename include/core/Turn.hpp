#pragma once

#include <string>

class Turn{
    private:
    std::string turn;
    
    public:
    void updateState(std::string);
    void showState();
    std::string getTurn();
};