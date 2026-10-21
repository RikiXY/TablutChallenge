#include "core/Turn.hpp"
#include <iostream>

void Turn::updateState(std::string rawJson){
    Translator translator;
    turn = translator.getTurn(rawJson);
}

void Turn::showState(){
    std::cout<<turn<<std::endl;
}

std::string Turn::getTurn(){
    return turn;
}