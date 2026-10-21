#include "core/Board.hpp"
#include "core/Translator.hpp"
#include <iostream>

void Board::updateState(std::string rawJson) {
    Translator translator;
    currentBoard = translator.getBoard(rawJson);
}

void Board::showState(){
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            std::cout<<currentBoard[i][j];
        }
        std::cout<<std::endl;
    }
}
