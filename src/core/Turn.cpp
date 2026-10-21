#include "core/Turn.hpp"
#include "core/Translator.hpp"
#include <iostream>

void Turn::updateState(std::string rawJson){
    Translator translator(rawJson);
    turn = translator.getTurn();
}

void Turn::showState(){
    std::cout<<turn<<std::endl;
}

std::string Turn::getTurn(){
    return turn;
}