#include "core/Turn.hpp"
#include "core/Translator.hpp"
#include <iostream>

void Turn::updateState(std::string rawJson){
    // ! Dovrei controllare che la il turn esista davvero nel JSON
    Translator translator(rawJson);
    turn = translator.getTurn();
}

void Turn::showState(){
    std::cout<<turn<<std::endl;
}

std::string Turn::getString(){
    return turn;
}