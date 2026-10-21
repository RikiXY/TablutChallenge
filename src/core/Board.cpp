#include "core/Board.hpp"
#include "core/Translator.hpp"
#include <iostream>

void Board::updateState(std::string rawJson) {
    Translator translator(rawJson);
    board = translator.getBoard();
}

void Board::showState(){
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            std::cout<<static_cast<int>(board[i][j]);
        }
        std::cout<<std::endl;
    }
}
