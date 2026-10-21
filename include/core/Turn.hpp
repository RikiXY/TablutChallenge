#pragma once

#include "Translator.hpp"

class Turn{
    private:
    std::string turn;
    
    public:
    void updateState(std::string rawJson);
    void showState();
    std::string getTurn();
};