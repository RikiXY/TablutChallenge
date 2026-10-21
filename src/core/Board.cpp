#include "core/Board.hpp"
#include "core/Translator.hpp"
#include <iostream>

void Board::updateState(std::string json) {
    Translator translator;
    currentBoard = translator.translateFromJson(json);
}

void Board::showState(){
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            std::cout<<currentBoard[i][j];
        }
        std::cout<<std::endl;
    }
}

BoardMatrix Board::getState(){
    return currentBoard;
}