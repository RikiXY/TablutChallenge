#include "core/Board.hpp"
#include "core/Translator.hpp"
#include <iostream>

void Board::updateState(std::string rawJson) {
    Translator translator(rawJson);
    board = translator.getBoard();
}

// void Board::showState(){
//     for(int i = 0; i < 9; i++){
//         for(int j = 0; j < 9; j++){
//             std::cout<<static_cast<int>(board[i][j]);
//         }
//         std::cout<<std::endl;
//     }
// }

void Board::showState() {
    std::cout<<"\n     a b c d e f g h i\n";
    std::cout<<"   +-------------------+\n";

    for (int i = 0; i < 9; i++) {
        std::cout<<" "<<i + 1<<" | ";
        for (int j = 0; j < 9; j++) {
            char symbol = '?';
            switch (board[i][j]) {
                case 0: symbol = '.'; break;
                case 1: symbol = 'W'; break;
                case 2: symbol = 'B'; break;
                case 3: symbol = 'K'; break;
                case 4: symbol = 'T'; break;
            }
            std::cout<<symbol<<' ';
        }
        std::cout << "| " << i + 1 << '\n';
    }
    std::cout << "   +-------------------+\n";
    std::cout << "     a b c d e f g h i\n\n";
    std::cout << "W = White | B = Black | K = King | . = Empty\n";
}

BoardMatrix Board::getBoardMatrix(){
    return  board;
}