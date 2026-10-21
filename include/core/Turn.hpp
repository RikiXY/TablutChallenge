#pragma once

#include "Translator.hpp"

class Turn{
    private:
    std::string currentTurn;
    
    public:
    void updateState(std::string rawJson);
    void showState();
};