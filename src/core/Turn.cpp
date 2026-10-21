#include "core/Turn.hpp"
#include <iostream>

void Turn::updateState(std::string rawJson){
    Translator translator;
    currentTurn = translator.getTurn(rawJson);
}

void Turn::showState(){
    std::cout<<currentTurn<<std::endl;
}