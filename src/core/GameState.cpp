#include "core/GameState.hpp"
#include "core/Translator.hpp"

void GameState::update(Board newBoard, Turn newTurn){
    
    currentBoard = newBoard;
    currentTurn = newTurn;

    if (currentTurn.getString() == "WHITEWIN" || currentTurn.getString() == "BLACKWIN" || currentTurn.getString() == "DRAW")
        finished = true;
    else 
        finished = false;
}

Board GameState::getBoard(){
    return currentBoard;
}

Turn GameState::getTurn(){
    return currentTurn;
}

bool GameState::isOver(){
    return finished;
}

std::string GameState::winner(){
    if(currentTurn.getString() == "WHITEWIN")
        return "WHITE";
    else if(currentTurn.getString() == "BLACKWIN")
        return "BLACK";
    else if(currentTurn.getString() == "DRAW")
        return "DRAW";
    else return "";
}