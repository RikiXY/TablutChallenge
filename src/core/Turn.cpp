#include "core/Turn.hpp"
#include "core/Translator.hpp"
#include <iostream>

void Turn::setTurn(std::string givenTurn) {
    turn = givenTurn;
}

void Turn::showState() {
    std::cout << turn << std::endl;
}

std::string Turn::getString() {
    return turn;
}